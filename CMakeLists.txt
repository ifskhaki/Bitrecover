cmake_minimum_required(VERSION 3.18)
project(bitrecover 
    VERSION 1.0.0
    DESCRIPTION "Multi-GPU Bitcoin Key Recovery Tool"
    LANGUAGES CUDA CXX
)

set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# CUDA detection
find_package(CUDA REQUIRED)
enable_language(CUDA)

# Auto-detect GPU architectures
if(UNIX)
    execute_process(
        COMMAND nvidia-smi --query-gpu=compute_cap --format=csv,noheader
        OUTPUT_VARIABLE GPU_ARCHS_RAW
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    if(GPU_ARCHS_RAW)
        string(REPLACE "\n" ";" GPU_ARCHS_LIST ${GPU_ARCHS_RAW})
        list(REMOVE_DUPLICATES GPU_ARCHS_LIST)
        
        set(CUDA_ARCHS "")
        foreach(ARCH ${GPU_ARCHS_LIST})
            string(REPLACE "." "" ARCH_CLEAN ${ARCH})
            list(APPEND CUDA_ARCHS "sm_${ARCH_CLEAN}")
        endforeach()
    endif()
endif()

# Default architectures if detection failed
if(NOT CUDA_ARCHS)
    set(CUDA_ARCHS "sm_70;sm_75;sm_80;sm_86;sm_89;sm_90")
endif()

message(STATUS "Building for CUDA architectures: ${CUDA_ARCHS}")

# Include directories

# OpenCL detection
find_package(OpenCL)
if(OpenCL_FOUND)
    message(STATUS "OpenCL found: ${OpenCL_INCLUDE_DIRS}")
    include_directories(${OpenCL_INCLUDE_DIRS})
    add_definitions(-DWE_HAVE_OPENCL)
else()
    message(WARNING "OpenCL NOT found. OpenCL support will be disabled.")
endif()

# Include directories
set(PROJECT_ROOT ${CMAKE_SOURCE_DIR})
# Use local paths - all legacy code is now in this repo
set(LEGACY_ROOT ${PROJECT_ROOT})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PROJECT_ROOT}
    ${PROJECT_ROOT}/AddressUtil
    ${PROJECT_ROOT}/KeyFinderLib
    ${PROJECT_ROOT}/CudaKeySearchDevice
    ${PROJECT_ROOT}/CLKeySearchDevice
    ${PROJECT_ROOT}/clUtil
    ${PROJECT_ROOT}/cudaMath
    ${PROJECT_ROOT}/cudaUtil
    ${PROJECT_ROOT}/CryptoUtil
    ${PROJECT_ROOT}/secp256k1lib
    ${PROJECT_ROOT}/util
    ${PROJECT_ROOT}/Logger
    ${PROJECT_ROOT}/CmdParse
    ${PROJECT_ROOT}/KeyFinder
    ${CUDA_INCLUDE_DIRS}
)

# Source files
set(BITRECOVER_SOURCES
    src/main.cpp
    src/BitrecoverEngine.cpp
    src/MultiGPUManager.cpp
    src/EmailNotifier.cpp
    src/StatusDisplay.cpp
    src/ConfigManager.cpp
    src/RandomKeyGenerator.cpp
)

# Legacy sources (from existing BitCrack codebase - now local to this repo)
set(LEGACY_SOURCES
    ${PROJECT_ROOT}/KeyFinderLib/KeyFinder.cpp
    ${PROJECT_ROOT}/CudaKeySearchDevice/CudaKeySearchDevice.cpp
    ${PROJECT_ROOT}/CudaKeySearchDevice/CudaKeySearchDevice.cu
    ${PROJECT_ROOT}/CudaKeySearchDevice/cudabridge.cu
    ${PROJECT_ROOT}/CudaKeySearchDevice/CudaDeviceKeys.cu
    ${PROJECT_ROOT}/CudaKeySearchDevice/CudaHashLookup.cu
    ${PROJECT_ROOT}/CudaKeySearchDevice/CudaAtomicList.cu
    ${PROJECT_ROOT}/AddressUtil/Base58.cpp
    ${PROJECT_ROOT}/AddressUtil/hash.cpp
    ${PROJECT_ROOT}/CryptoUtil/sha256.cpp
    ${PROJECT_ROOT}/CryptoUtil/ripemd160.cpp
    ${PROJECT_ROOT}/CryptoUtil/Rng.cpp
    ${PROJECT_ROOT}/CryptoUtil/hash.cpp
    ${PROJECT_ROOT}/secp256k1lib/secp256k1.cpp
    ${PROJECT_ROOT}/util/util.cpp
    ${PROJECT_ROOT}/Logger/Logger.cpp
    ${PROJECT_ROOT}/CmdParse/CmdParse.cpp
    ${PROJECT_ROOT}/KeyFinder/DeviceManager.cpp
    ${PROJECT_ROOT}/KeyFinder/ConfigFile.cpp
)

# OpenCL Sources
set(OPENCL_SOURCES "")
if(OpenCL_FOUND)
    # EmbedCL tool
    add_executable(embedcl embedcl/main.cpp)

    set(CL_KERNELS
        ${PROJECT_ROOT}/clMath/ripemd160.cl
        ${PROJECT_ROOT}/clMath/secp256k1.cl
        ${PROJECT_ROOT}/clMath/sha256.cl
        ${PROJECT_ROOT}/CLKeySearchDevice/keysearch.cl
    )
    
    set(BITCRACK_CL ${CMAKE_BINARY_DIR}/bitcrack.cl)
    set(BITCRACK_CL_CPP ${CMAKE_BINARY_DIR}/bitcrack_cl.cpp)

    # Concatenate CL files
    if(WIN32)
        # Use powershell for reliable concatenation on Windows
        add_custom_command(
            OUTPUT ${BITCRACK_CL}
            COMMAND powershell -Command "Get-Content '${PROJECT_ROOT}/clMath/ripemd160.cl', '${PROJECT_ROOT}/clMath/secp256k1.cl', '${PROJECT_ROOT}/clMath/sha256.cl', '${PROJECT_ROOT}/CLKeySearchDevice/keysearch.cl' | Set-Content '${BITCRACK_CL}'"
            DEPENDS ${CL_KERNELS}
            COMMENT "Concatenating OpenCL kernels"
        )
    else()
        add_custom_command(
            OUTPUT ${BITCRACK_CL}
            COMMAND cat ${PROJECT_ROOT}/clMath/ripemd160.cl ${PROJECT_ROOT}/clMath/secp256k1.cl ${PROJECT_ROOT}/clMath/sha256.cl ${PROJECT_ROOT}/CLKeySearchDevice/keysearch.cl > ${BITCRACK_CL}
            DEPENDS ${CL_KERNELS}
            COMMENT "Concatenating OpenCL kernels"
        )
    endif()

    # Run embedcl
    add_custom_command(
        OUTPUT ${BITCRACK_CL_CPP}
        COMMAND $<TARGET_FILE:embedcl> ${BITCRACK_CL} ${BITCRACK_CL_CPP} _bitcrack_cl
        DEPENDS embedcl ${BITCRACK_CL}
        COMMENT "Embedding OpenCL kernels into C++"
    )

    list(APPEND OPENCL_SOURCES
        ${PROJECT_ROOT}/CLKeySearchDevice/CLKeySearchDevice.cpp
        ${PROJECT_ROOT}/clUtil/clContext.cpp
        ${PROJECT_ROOT}/clUtil/clUtil.cpp
        ${PROJECT_ROOT}/clUtil/clerrors.cpp
        ${BITCRACK_CL_CPP}
    )
endif()

# Separate CUDA and C++ sources
set(CUDA_SOURCES "")
set(CPP_SOURCES "")

foreach(SRC ${BITRECOVER_SOURCES} ${LEGACY_SOURCES} ${OPENCL_SOURCES})
    if(SRC MATCHES "\\.cu$")
        list(APPEND CUDA_SOURCES ${SRC})
    else()
        list(APPEND CPP_SOURCES ${SRC})
    endif()
endforeach()

# Executable
add_executable(bitrecover ${CPP_SOURCES} ${CUDA_SOURCES})

# CUDA properties
set_target_properties(bitrecover PROPERTIES
    CUDA_ARCHITECTURES "${CUDA_ARCHS}"
    CUDA_SEPARABLE_COMPILATION ON
    CUDA_RESOLVE_DEVICE_SYMBOLS ON
)

# Compiler flags
target_compile_options(bitrecover PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-O3 -march=native -Wall -Wextra>
    $<$<COMPILE_LANGUAGE:CUDA>:-O3 --use_fast_math>
)

# Link libraries
target_link_libraries(bitrecover PRIVATE
    ${CUDA_LIBRARIES}
    ${CUDA_CUDART_LIBRARY}
    pthread
)

if(OpenCL_FOUND)
    target_link_libraries(bitrecover PRIVATE OpenCL::OpenCL)
endif()

# AddrGen Tool
set(ADDRGEN_SOURCES
    tools/AddrGen/main.cpp
    ${PROJECT_ROOT}/secp256k1lib/secp256k1.cpp
    ${PROJECT_ROOT}/util/util.cpp
    ${PROJECT_ROOT}/AddressUtil/Base58.cpp
    ${PROJECT_ROOT}/AddressUtil/hash.cpp
    ${PROJECT_ROOT}/CryptoUtil/sha256.cpp
    ${PROJECT_ROOT}/CryptoUtil/ripemd160.cpp
    ${PROJECT_ROOT}/CryptoUtil/Rng.cpp
    ${PROJECT_ROOT}/CryptoUtil/hash.cpp
    ${PROJECT_ROOT}/CmdParse/CmdParse.cpp
)

add_executable(addrgen ${ADDRGEN_SOURCES})
target_link_libraries(addrgen PRIVATE pthread)

# Installation
install(TARGETS bitrecover addrgen DESTINATION bin)
install(DIRECTORY scripts/ DESTINATION share/bitrecover/scripts)
install(DIRECTORY config/ DESTINATION share/bitrecover/config)
install(FILES README.md LICENSE DESTINATION share/bitrecover)

# Print build info
message(STATUS "")
message(STATUS "=== Bitrecover Build Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "CUDA architectures: ${CUDA_ARCHS}")
message(STATUS "OpenCL Support: ${OpenCL_FOUND}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "CUDA standard: ${CMAKE_CUDA_STANDARD}")
message(STATUS "=====================================")
message(STATUS "")

