#include "MultiGPUManager.h"
#ifdef WE_HAVE_OPENCL
#include "CLKeySearchDevice.h"
#endif
#include "RandomKeyGenerator.h"
#include "Logger.h"
#include "AddressUtil.h"
#include "KeySearchTypes.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <set>
#include <cstring>
#include <map>

// Global state for callbacks (KeyFinder uses C-style function pointers, not std::function)
static std::map<int, MultiGPUManager*> g_managerInstances;
static std::mutex g_callbackMutex;
static int g_currentWorkerIndex = -1;
static MultiGPUManager* g_currentManager = nullptr;

// Static callback wrappers for KeyFinder's C-style function pointer API
static void staticResultCallback(KeySearchResult result) {
    std::lock_guard<std::mutex> lock(g_callbackMutex);
    if (g_currentManager && g_currentWorkerIndex >= 0) {
        g_currentManager->handleResult(result, g_currentWorkerIndex);
    }
}

static void staticStatusCallback(KeySearchStatus status) {
    std::lock_guard<std::mutex> lock(g_callbackMutex);
    if (g_currentManager && g_currentWorkerIndex >= 0) {
        g_currentManager->handleStatus(status, g_currentWorkerIndex);
    }
}

MultiGPUManager::MultiGPUManager() {
}

MultiGPUManager::~MultiGPUManager() {
    stopAll();
}

bool MultiGPUManager::initializeAllGPUs(const std::string& targetsFile,
                                       const bitrecover::Config::GPUConfig& gpuConfig,
                                       const bitrecover::Config::SearchConfig& searchConfig) {
    try {
        std::vector<DeviceManager::DeviceInfo> devices = DeviceManager::getDevices();
        
        if (devices.empty()) {
            Logger::log(LogLevel::Error, "No GPU devices found");
            return false;
        }
        
        // Filter devices based on gpuConfig
        std::vector<DeviceManager::DeviceInfo> selectedDevices;
        if (gpuConfig.useAllGPUs || gpuConfig.gpuIds.empty()) {
            selectedDevices = devices;
        } else {
            for (int id : gpuConfig.gpuIds) {
                if (id >= 0 && id < static_cast<int>(devices.size())) {
                    selectedDevices.push_back(devices[id]);
                }
            }
        }
        
        if (selectedDevices.empty()) {
            Logger::log(LogLevel::Error, "No valid GPU devices selected");
            return false;
        }
        
        // Load targets as vector of strings (addresses)
        std::vector<std::string> targetAddresses;
        Logger::log(LogLevel::Info, "Opening targets file: " + targetsFile);
        std::ifstream file(targetsFile);
        if (file.is_open()) {
            std::string line;
            int lineCount = 0;
            while (std::getline(file, line)) {
                lineCount++;
                // Remove whitespace
                line.erase(0, line.find_first_not_of(" \t\r\n"));
                if (!line.empty() && line.find_last_not_of(" \t\r\n") != std::string::npos) {
                    line.erase(line.find_last_not_of(" \t\r\n") + 1);
                }
                
                if (!line.empty()) {
                    targetAddresses.push_back(line);
                }
            }
            file.close();
            Logger::log(LogLevel::Info, "Read " + std::to_string(lineCount) + " lines, found " + 
                        std::to_string(targetAddresses.size()) + " addresses");
        } else {
            Logger::log(LogLevel::Error, "Could not open targets file: " + targetsFile);
            return false;
        }
        
        if (targetAddresses.empty()) {
            Logger::log(LogLevel::Error, "No target addresses loaded from file: " + targetsFile);
            return false;
        }
        
        // Initialize workers
        RandomKeyGenerator rng;
        for (size_t i = 0; i < selectedDevices.size(); ++i) {
            const auto& deviceInfo = selectedDevices[i];
            
            GPUWorker worker;
            worker.gpuId = deviceInfo.id;
            worker.running = false;
            worker.keysProcessed = 0;
            worker.speedMKeysPerSec = 0.0;
            
            // Get GPU parameters with defaults
            int threads = gpuConfig.threadsPerBlock > 0 ? gpuConfig.threadsPerBlock : 256;
            int pointsPerThread = gpuConfig.pointsPerThread > 0 ? gpuConfig.pointsPerThread : 256;
            int blocks = gpuConfig.blocks > 0 ? gpuConfig.blocks : 0; // 0 = auto
            
            // Create device based on type
            if (deviceInfo.type == DeviceManager::DeviceType::CUDA) {
                worker.device = new CudaKeySearchDevice(deviceInfo.id, threads, pointsPerThread, blocks);
            } else if (deviceInfo.type == DeviceManager::DeviceType::OpenCL) {
#ifdef WE_HAVE_OPENCL
                worker.device = new CLKeySearchDevice(deviceInfo.id, threads, pointsPerThread, blocks);
#else
                Logger::log(LogLevel::Warning, "OpenCL support not compiled. Skipping device " + std::to_string(deviceInfo.id));
                continue;
#endif
            } else {
                Logger::log(LogLevel::Warning, "Unknown device type");
                continue;
            }
            
            // Generate random start key for this GPU
            secp256k1::uint256 startKey = rng.generateRandom256ForGPU(i, selectedDevices.size());
            unsigned int maxWords[8] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
                                        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
            secp256k1::uint256 endKey(maxWords, secp256k1::uint256::LittleEndian);
            secp256k1::uint256 stride(1);
            
            int compression = 0; // UNCOMPRESSED
            if (searchConfig.compression == "COMPRESSED") {
                compression = 1;
            } else if (searchConfig.compression == "BOTH") {
                compression = 2;
            }
            
            worker.finder = new KeyFinder(startKey, endKey, compression, worker.device, stride);
            worker.finder->setTargets(targetAddresses);
            
            workers_.push_back(std::move(worker));
            
            Logger::log(LogLevel::Info, 
                "Initialized GPU " + std::to_string(deviceInfo.id) + ": " + deviceInfo.name);
        }
        
        return !workers_.empty();
    } catch (const std::exception& e) {
        Logger::log(LogLevel::Error, "Failed to initialize GPUs: " + std::string(e.what()));
        return false;
    } catch (const DeviceManager::DeviceManagerException& e) {
        Logger::log(LogLevel::Error, "Failed to initialize GPUs: " + e.msg);
        return false;
    }
}

void MultiGPUManager::startParallelSearch(const bitrecover::Config::SearchConfig& /*config*/) {
    stopRequested_ = false;
    
    for (size_t i = 0; i < workers_.size(); ++i) {
        auto& worker = workers_[i];
        if (!worker.running) {
            worker.running = true;
            worker.thread = std::make_unique<std::thread>(&MultiGPUManager::workerThread, this, static_cast<int>(i));
        }
    }
}

void MultiGPUManager::workerThread(int workerIndex) {
    if (workerIndex < 0 || workerIndex >= static_cast<int>(workers_.size())) {
        return;
    }
    
    GPUWorker& worker = workers_[workerIndex];
    
    try {
        // Set up global state for static callbacks
        {
            std::lock_guard<std::mutex> lock(g_callbackMutex);
            g_currentManager = this;
            g_currentWorkerIndex = workerIndex;
        }
        
        // Initialize the finder
        worker.finder->init();
        
        // Set static callbacks (KeyFinder uses C-style function pointers)
        worker.finder->setResultCallback(staticResultCallback);
        worker.finder->setStatusCallback(staticStatusCallback);
        
        // Run the search - this is blocking
        worker.finder->run();
        
    } catch (const std::exception& e) {
        Logger::log(LogLevel::Error, "GPU " + std::to_string(worker.gpuId) + " error: " + e.what());
    }
    
    worker.running = false;
}

// Called from static callback
void MultiGPUManager::handleResult(const KeySearchResult& result, int workerIndex) {
    if (workerIndex < 0 || workerIndex >= static_cast<int>(workers_.size())) {
        return;
    }
    
    int gpuId = workers_[workerIndex].gpuId;
    
    if (resultCallback_) {
        resultCallback_(result, gpuId);
    }
}

// Called from static callback
void MultiGPUManager::handleStatus(const KeySearchStatus& status, int workerIndex) {
    if (workerIndex < 0 || workerIndex >= static_cast<int>(workers_.size())) {
        return;
    }
    
    GPUWorker& w = workers_[workerIndex];
    w.keysProcessed = status.total;
    w.speedMKeysPerSec = status.speed / 1000000.0;
    
    w.stats.gpuId = w.gpuId;
    w.stats.keysProcessed = w.keysProcessed;
    w.stats.speedMKeysPerSec = w.speedMKeysPerSec;
    w.stats.isRunning = w.running;
    w.stats.utilizationPercent = 95.0;
    
    if (statusCallback_) {
        statusCallback_(w.stats);
    }
}

void MultiGPUManager::stopAll() {
    stopRequested_ = true;
    
    for (auto& worker : workers_) {
        if (worker.finder) {
            worker.finder->stop();
        }
        worker.running = false;
    }
    
    for (auto& worker : workers_) {
        if (worker.thread && worker.thread->joinable()) {
            worker.thread->join();
        }
    }
    
    for (auto& worker : workers_) {
        delete worker.finder;
        worker.finder = nullptr;
        // Note: KeySearchDevice should have virtual destructor but doesn't
        // This is a design issue in the original library
        worker.device = nullptr; // Don't delete - let KeyFinder handle it
    }
    
    workers_.clear();
}

bool MultiGPUManager::isAnyRunning() const {
    for (const auto& worker : workers_) {
        if (worker.running) {
            return true;
        }
    }
    return false;
}

std::vector<bitrecover::GPUStats> MultiGPUManager::getStats() const {
    std::lock_guard<std::mutex> lock(statsMutex_);
    std::vector<bitrecover::GPUStats> stats;
    for (const auto& worker : workers_) {
        stats.push_back(worker.stats);
    }
    return stats;
}

void MultiGPUManager::setResultCallback(std::function<void(const ::KeySearchResult&, int)> callback) {
    resultCallback_ = callback;
}

void MultiGPUManager::setStatusCallback(std::function<void(const bitrecover::GPUStats&)> callback) {
    statusCallback_ = callback;
}

std::string MultiGPUManager::getDeviceTypeName(const DeviceManager::DeviceInfo& device) {
    if (device.type == DeviceManager::DeviceType::CUDA) {
        return "CUDA";
    } else if (device.type == DeviceManager::DeviceType::OpenCL) {
        return "OpenCL";
    }
    return "Unknown";
}
