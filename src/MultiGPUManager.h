#ifndef MULTI_GPU_MANAGER_H
#define MULTI_GPU_MANAGER_H

#include "bitrecover/types.h"
#include "KeySearchDevice.h"
#include "KeySearchTypes.h"
#include "KeyFinder.h"
#include "DeviceManager.h"
#include "CudaKeySearchDevice.h"
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <mutex>
#include <set>
#include <memory>

class MultiGPUManager {
public:
    MultiGPUManager();
    ~MultiGPUManager();

    bool initializeAllGPUs(const std::string& targetsFile,
                          const bitrecover::Config::GPUConfig& gpuConfig,
                          const bitrecover::Config::SearchConfig& searchConfig);
    
    void startParallelSearch(const bitrecover::Config::SearchConfig& config);
    void stopAll();
    
    bool isAnyRunning() const;
    std::vector<bitrecover::GPUStats> getStats() const;
    
    void setResultCallback(std::function<void(const ::KeySearchResult&, int)> callback);
    void setStatusCallback(std::function<void(const bitrecover::GPUStats&)> callback);

private:
    struct GPUWorker {
        int gpuId = 0;
        KeySearchDevice* device = nullptr;
        KeyFinder* finder = nullptr;
        std::unique_ptr<std::thread> thread;
        bool running = false;
        uint64_t keysProcessed = 0;
        double speedMKeysPerSec = 0.0;
        bitrecover::GPUStats stats;
        
        GPUWorker() = default;
        GPUWorker(GPUWorker&& other) noexcept 
            : gpuId(other.gpuId)
            , device(other.device)
            , finder(other.finder)
            , thread(std::move(other.thread))
            , running(other.running)
            , keysProcessed(other.keysProcessed)
            , speedMKeysPerSec(other.speedMKeysPerSec)
            , stats(other.stats)
        {
            other.device = nullptr;
            other.finder = nullptr;
        }
        GPUWorker& operator=(GPUWorker&& other) noexcept {
            if (this != &other) {
                gpuId = other.gpuId;
                device = other.device;
                finder = other.finder;
                thread = std::move(other.thread);
                running = other.running;
                keysProcessed = other.keysProcessed;
                speedMKeysPerSec = other.speedMKeysPerSec;
                stats = other.stats;
                other.device = nullptr;
                other.finder = nullptr;
            }
            return *this;
        }
        // Prevent copying
        GPUWorker(const GPUWorker&) = delete;
        GPUWorker& operator=(const GPUWorker&) = delete;
    };
    
    std::vector<GPUWorker> workers_;
    std::function<void(const ::KeySearchResult&, int)> resultCallback_;
    std::function<void(const bitrecover::GPUStats&)> statusCallback_;
    mutable std::mutex statsMutex_;
    std::atomic<bool> stopRequested_{false};
    
    void workerThread(int workerIndex, const bitrecover::Config::SearchConfig& config);
    std::string getDeviceTypeName(const DeviceManager::DeviceInfo& device);
};

#endif // MULTI_GPU_MANAGER_H
