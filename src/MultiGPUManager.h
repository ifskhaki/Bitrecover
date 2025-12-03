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
        int gpuId;
        KeySearchDevice* device;
        KeyFinder* finder;
        std::thread thread;
        std::atomic<bool> running;
        std::atomic<uint64_t> keysProcessed;
        std::atomic<double> speedMKeysPerSec;
        bitrecover::GPUStats stats;
    };
    
    std::vector<GPUWorker> workers_;
    std::function<void(const ::KeySearchResult&, int)> resultCallback_;
    std::function<void(const bitrecover::GPUStats&)> statusCallback_;
    std::mutex statsMutex_;
    
    void workerThread(int gpuId, const bitrecover::Config::SearchConfig& config);
    std::string getDeviceTypeName(const DeviceManager::DeviceInfo& device);
};

#endif // MULTI_GPU_MANAGER_H

