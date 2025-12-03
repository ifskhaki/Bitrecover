#ifndef BITRECOVER_ENGINE_H
#define BITRECOVER_ENGINE_H

#include "bitrecover/types.h"
#include <string>
#include <memory>

class ConfigManager;
class MultiGPUManager;
class EmailNotifier;
class StatusDisplay;

class BitrecoverEngine {
public:
    BitrecoverEngine();
    ~BitrecoverEngine();

    bool initialize(const std::string& configFile);
    int run();
    void stop();

private:
    bool initialized_;
    bitrecover::Config config_;
    bitrecover::SystemInfo systemInfo_;
    
    std::unique_ptr<ConfigManager> configManager_;
    std::unique_ptr<MultiGPUManager> gpuManager_;
    std::unique_ptr<EmailNotifier> emailNotifier_;
    std::unique_ptr<StatusDisplay> statusDisplay_;
    
    bool loadConfiguration(const std::string& configFile);
    bool gatherSystemInfo();
    void setupCallbacks();
    void sendStartupNotification();
};

#endif // BITRECOVER_ENGINE_H

