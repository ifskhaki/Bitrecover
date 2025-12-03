#ifndef BITRECOVER_TYPES_H
#define BITRECOVER_TYPES_H

#include <string>
#include <vector>
#include <cstdint>

namespace bitrecover {

struct GPUInfo {
    int id;
    std::string name;
    uint64_t memory;  // bytes
    int computeUnits;
    int cudaMajor;
    int cudaMinor;
    std::string type;  // "CUDA" or "OpenCL"
};

struct SystemInfo {
    std::string hostname;
    std::string os;
    std::vector<GPUInfo> gpus;
};

struct MatchResult {
    std::string address;
    std::string privateKeyHex;
    std::string wif;
    int gpuId;
    std::string timestamp;
};

struct GPUStats {
    int gpuId;
    std::string name;
    uint64_t keysProcessed;
    double speedMKeysPerSec;
    double utilizationPercent;
    bool isRunning;
    std::string status;
};

struct Config {
    struct EmailConfig {
        bool enabled;
        bool sendStartupEmail;   // NEW: Control startup notification separately
        bool sendMatchEmail;     // NEW: Control match notification separately
        std::string smtpServer;
        int smtpPort;
        std::string username;
        std::string password;
        std::vector<std::string> recipients;
    } email;

    struct GPUConfig {
        bool useAllGPUs;
        std::vector<int> gpuIds;
        bool autoOptimize;
        int threadsPerBlock;
        int blocks;
        int pointsPerThread;
    } gpu;

    struct SearchConfig {
        std::string targetsFile;
        std::string outputFile;
        std::string compression;
        bool random256;
        int statusIntervalMs;
        std::string checkpointFile;
        int checkpointIntervalMs;
    } search;

    struct DisplayConfig {
        bool realTime;
        int updateIntervalMs;
        bool showGpuDetails;
        bool clearScreen;
    } display;
};

} // namespace bitrecover

#endif // BITRECOVER_TYPES_H

