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
        
        // Load targets
        std::set<KeySearchTarget> targets;
        std::ifstream file(targetsFile);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                // Remove whitespace
                line.erase(0, line.find_first_not_of(" \t\r\n"));
                line.erase(line.find_last_not_of(" \t\r\n") + 1);
                
                if (!line.empty()) {
                    // Convert address to hash160
                    unsigned int hash[5];
                    Base58::toHash160(line, hash);
                    KeySearchTarget target(hash);
                    targets.insert(target);
                }
            }
            file.close();
        } else {
            Logger::log(LogLevel::Warning, "Could not open targets file: " + targetsFile);
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
            
            // Create device
            if (deviceInfo.type == DeviceManager::DeviceType::CUDA) {
                worker.device = new CudaKeySearchDevice(deviceInfo.id);
            } else if (deviceInfo.type == DeviceManager::DeviceType::OpenCL) {
#ifdef WE_HAVE_OPENCL
                worker.device = new CLKeySearchDevice(deviceInfo.id, config.gpu.threadsPerBlock, config.gpu.pointsPerThread, config.gpu.blocks);
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
            // Set endKey to maximum value (all 0xFFFFFFFF)
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
            worker.finder->setTargets(targets);
            
            workers_.push_back(worker);
            
            Logger::log(LogLevel::Info, 
                "Initialized GPU " + std::to_string(deviceInfo.id) + ": " + deviceInfo.name);
        }
        
        return !workers_.empty();
    } catch (const std::exception& e) {
        Logger::log(LogLevel::Error, "Failed to initialize GPUs: " + std::string(e.what()));
        return false;
    }
}

void MultiGPUManager::startParallelSearch(const bitrecover::Config::SearchConfig& config) {
    for (auto& worker : workers_) {
        if (!worker.running) {
            worker.running = true;
            worker.thread = std::thread(&MultiGPUManager::workerThread, this, worker.gpuId, config);
        }
    }
}

void MultiGPUManager::workerThread(int gpuId, const bitrecover::Config::SearchConfig& config) {
    auto workerIt = std::find_if(workers_.begin(), workers_.end(),
                                [gpuId](const GPUWorker& w) { return w.gpuId == gpuId; });
    if (workerIt == workers_.end()) {
        return;
    }
    
    GPUWorker& worker = *workerIt;
    auto startTime = std::chrono::steady_clock::now();
    auto lastStatusTime = startTime;
    
    while (worker.running) {
        try {
            worker.finder->doStep();
            worker.keysProcessed++;
            
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
            if (elapsed > 0) {
                worker.speedMKeysPerSec = (worker.keysProcessed * 1000.0) / elapsed / 1000000.0;
            }
            
            // Update stats periodically
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastStatusTime).count() 
                >= config.statusIntervalMs) {
                worker.stats.gpuId = worker.gpuId;
                worker.stats.keysProcessed = worker.keysProcessed;
                worker.stats.speedMKeysPerSec = worker.speedMKeysPerSec;
                worker.stats.isRunning = worker.running;
                worker.stats.utilizationPercent = 95.0; // Simplified
                
                if (statusCallback_) {
                    statusCallback_(worker.stats);
                }
                
                lastStatusTime = now;
            }
            
            // Check for results
            std::vector<::KeySearchResult> results = worker.finder->getResults();
            for (const auto& result : results) {
                if (resultCallback_) {
                    resultCallback_(result, worker.gpuId);
                }
            }
            
        } catch (const std::exception& e) {
            Logger::log(LogLevel::Error, "GPU " + std::to_string(gpuId) + " error: " + e.what());
            break;
        }
    }
}

void MultiGPUManager::stopAll() {
    for (auto& worker : workers_) {
        worker.running = false;
        if (worker.thread.joinable()) {
            worker.thread.join();
        }
        delete worker.finder;
        delete worker.device;
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

