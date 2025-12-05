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
        
        // Load targets as vector of strings (addresses)
        std::vector<std::string> targetAddresses;
        std::ifstream file(targetsFile);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                // Remove whitespace
                line.erase(0, line.find_first_not_of(" \t\r\n"));
                line.erase(line.find_last_not_of(" \t\r\n") + 1);
                
                if (!line.empty()) {
                    targetAddresses.push_back(line);
                }
            }
            file.close();
        } else {
            Logger::log(LogLevel::Warning, "Could not open targets file: " + targetsFile);
        }
        
        if (targetAddresses.empty()) {
            Logger::log(LogLevel::Warning, "No target addresses loaded");
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
                // CudaKeySearchDevice(int device, int threads, int pointsPerThread, int blocks = 0)
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
            
            // KeyFinder(startKey, endKey, compression, device, stride)
            worker.finder = new KeyFinder(startKey, endKey, compression, worker.device, stride);
            
            // Set targets using vector<string> overload
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

void MultiGPUManager::startParallelSearch(const bitrecover::Config::SearchConfig& config) {
    stopRequested_ = false;
    
    for (size_t i = 0; i < workers_.size(); ++i) {
        auto& worker = workers_[i];
        if (!worker.running) {
            worker.running = true;
            // Pass workerIndex instead of gpuId to find the right worker
            worker.thread = std::make_unique<std::thread>(&MultiGPUManager::workerThread, this, static_cast<int>(i), config);
        }
    }
}

void MultiGPUManager::workerThread(int workerIndex, const bitrecover::Config::SearchConfig& config) {
    if (workerIndex < 0 || workerIndex >= static_cast<int>(workers_.size())) {
        return;
    }
    
    GPUWorker& worker = workers_[workerIndex];
    
    try {
        // Initialize the finder
        worker.finder->init();
        
        // Set up callbacks for the KeyFinder
        // Note: KeyFinder's run() is blocking and handles its own loop
        // We use callbacks set via setResultCallback and setStatusCallback
        
        worker.finder->setResultCallback([this, workerIndex](KeySearchResult result) {
            if (resultCallback_) {
                resultCallback_(result, workers_[workerIndex].gpuId);
            }
        });
        
        worker.finder->setStatusCallback([this, workerIndex](KeySearchStatus status) {
            GPUWorker& w = workers_[workerIndex];
            w.keysProcessed = status.total;
            w.speedMKeysPerSec = status.speed / 1000000.0; // Convert to MKeys/s
            
            // Update stats
            w.stats.gpuId = w.gpuId;
            w.stats.keysProcessed = w.keysProcessed;
            w.stats.speedMKeysPerSec = w.speedMKeysPerSec;
            w.stats.isRunning = w.running;
            w.stats.utilizationPercent = 95.0; // Approximate
            
            if (statusCallback_) {
                statusCallback_(w.stats);
            }
        });
        
        // Run the search - this is blocking
        worker.finder->run();
        
    } catch (const std::exception& e) {
        Logger::log(LogLevel::Error, "GPU " + std::to_string(worker.gpuId) + " error: " + e.what());
    }
    
    worker.running = false;
}

void MultiGPUManager::stopAll() {
    stopRequested_ = true;
    
    // Stop all finders
    for (auto& worker : workers_) {
        if (worker.finder) {
            worker.finder->stop();
        }
        worker.running = false;
    }
    
    // Join all threads
    for (auto& worker : workers_) {
        if (worker.thread && worker.thread->joinable()) {
            worker.thread->join();
        }
    }
    
    // Clean up
    for (auto& worker : workers_) {
        delete worker.finder;
        worker.finder = nullptr;
        delete worker.device;
        worker.device = nullptr;
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
