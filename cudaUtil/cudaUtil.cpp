#include "cudaUtil.h"


cuda::CudaDeviceInfo cuda::getDeviceInfo(int device)
{
	cuda::CudaDeviceInfo devInfo;

	cudaDeviceProp properties;
	cudaError_t err = cudaSuccess;

	err = cudaSetDevice(device);

	if(err) {
		throw cuda::CudaException(err);
	}

	err = cudaGetDeviceProperties(&properties, device);
	
	if(err) {
		throw cuda::CudaException(err);
	}

	devInfo.id = device;
	devInfo.major = properties.major;
	devInfo.minor = properties.minor;
	devInfo.mpCount = properties.multiProcessorCount;
	devInfo.mem = properties.totalGlobalMem;
	devInfo.name = std::string(properties.name);

	int cores = 0;
	int multiprocessorCount = properties.multiProcessorCount;
	(int)multiprocessorCount; // suppress unused warning if not used
	int major = properties.major;
	int minor = properties.minor;

	switch(major) {
	case 1:
		cores = 8; // Tesla
		break;
	case 2:
		// Fermi
		cores = (minor == 0) ? 32 : 48;
		break;
	case 3:
		// Kepler
		cores = 192;
		break;
	case 5:
		// Maxwell
		cores = 128;
		break;
	case 6:
		// Pascal
		cores = (minor == 1 || minor == 2) ? 128 : 64;
		break;
	case 7:
		// Volta (7.0) / Turing (7.5)
		cores = 64;
		break;
	case 8:
		// Ampere / Ada
		if(minor == 0) {
			// A100
			cores = 64;
		} else if(minor == 6) {
			// RTX 30xx
			cores = 128;
		} else if(minor == 9) {
			// RTX 40xx
			cores = 128;
		} else {
			cores = 64;
		}
		break;
	case 9:
		// Hopper (H100)
		cores = 128;
		break;
	default:
		// Fallback: approximate cores per SM from maxThreadsPerMultiprocessor when available
		#if CUDA_VERSION >= 11000
		{
			int cudaCoresPerSM = 0;
			if(cudaDeviceGetAttribute(&cudaCoresPerSM, cudaDevAttrMaxThreadsPerMultiProcessor, device) == cudaSuccess && cudaCoresPerSM >= 32) {
				cores = cudaCoresPerSM / 32;
			} else {
				cores = 64; // Safe default for modern GPUs
			}
		}
		#else
		cores = 64; // Default for unknown modern architectures
		#endif
		break;
	}
	devInfo.cores = cores;

	return devInfo;
}


std::vector<cuda::CudaDeviceInfo> cuda::getDevices()
{
	int count = getDeviceCount();

	std::vector<CudaDeviceInfo> devList;

	for(int device = 0; device < count; device++) {
		devList.push_back(getDeviceInfo(device));
	}

	return devList;
}

int cuda::getDeviceCount()
{
	int count = 0;

	cudaError_t err = cudaGetDeviceCount(&count);

    if(err) {
        throw cuda::CudaException(err);
    }

	return count;
}