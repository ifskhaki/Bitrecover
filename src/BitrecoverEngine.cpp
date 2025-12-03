#include "BitrecoverEngine.h"
#include "ConfigManager.h"
#include "MultiGPUManager.h"
#include "EmailNotifier.h"
#include "StatusDisplay.h"
#include "Logger.h"
#include "DeviceManager.h"
#include "util.h"
#include "AddressUtil.h"
#include "KeySearchDevice.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/utsname.h>
#endif

BitrecoverEngine::BitrecoverEngine() : initialized_(false) {
    configManager_ = std::make_unique<ConfigManager>();
    statusDisplay_ = std::make_unique<StatusDisplay>();
}

BitrecoverEngine::~BitrecoverEngine() {
    stop();
}

bool BitrecoverEngine::initialize(const std::string& configFile) {
    if (initialized_) {
        Logger::log(LogLevel::Warning, "Engine already initialized");
        return true;
    }
    
    // Load configuration
    if (!loadConfiguration(configFile)) {
        Logger::log(LogLevel::Warning, "Using default configuration");
    }
    
    config_ = configManager_->getConfig();
    
    // Gather system information
    if (!gatherSystemInfo()) {
        Logger::log(LogLevel::Error, "Failed to gather system information");
        return false;
    }
    
    // Create email notifier
    emailNotifier_ = std::make_unique<EmailNotifier>(config_.email);
    
    // Create GPU manager
    gpuManager_ = std::make_unique<MultiGPUManager>();
    
    // Initialize GPUs
    if (!gpuManager_->initializeAllGPUs(
            config_.search.targetsFile,
            config_.gpu,
            config_.search)) {
        Logger::log(LogLevel::Error, "Failed to initialize GPUs");
        return false;
    }
    
    // Setup callbacks
    setupCallbacks();
    
    // Send startup notification
    sendStartupNotification();
    
    initialized_ = true;
    Logger::log(LogLevel::Info, "Bitrecover engine initialized successfully");
    
    return true;
}

bool BitrecoverEngine::loadConfiguration(const std::string& configFile) {
    return configManager_->loadFromFile(configFile);
}

bool BitrecoverEngine::gatherSystemInfo() {
    systemInfo_.hostname = configManager_->getHostname();
    
#ifdef _WIN32
    systemInfo_.os = "Windows";
#else
    struct utsname unameInfo;
    if (uname(&unameInfo) == 0) {
        systemInfo_.os = std::string(unameInfo.sysname) + " " + unameInfo.release;
    } else {
        systemInfo_.os = "Linux";
    }
#endif
    
    // Get GPU information
    try {
        std::vector<DeviceManager::DeviceInfo> devices = DeviceManager::getDevices();
        for (const auto& dev : devices) {
            bitrecover::GPUInfo gpuInfo;
            gpuInfo.id = dev.id;
            gpuInfo.name = dev.name;
            gpuInfo.memory = dev.memory;
            gpuInfo.computeUnits = dev.computeUnits;
            gpuInfo.cudaMajor = dev.cudaMajor;
            gpuInfo.cudaMinor = dev.cudaMinor;
            if (dev.type == DeviceManager::DeviceType::CUDA) {
                gpuInfo.type = "CUDA";
            } else {
                gpuInfo.type = "OpenCL";
                gpuInfo.cudaMajor = 0;
                gpuInfo.cudaMinor = 0;
            }
            systemInfo_.gpus.push_back(gpuInfo);
        }
    } catch (const std::exception& e) {
        Logger::log(LogLevel::Warning, "Failed to get GPU info: " + std::string(e.what()));
    }
    
    return true;
}

void BitrecoverEngine::setupCallbacks() {
    // Result callback - handle matches
    gpuManager_->setResultCallback([this](const ::KeySearchResult& result, int gpuId) {
        // Convert to WIF
        std::string wif = Address::privateKeyToWIF(result.privateKey, result.compressed);
        
        // Convert private key to hex string
        std::string privateKeyHex = result.privateKey.toString(16);
        
        // Save to file
        std::string line = result.address + " " + 
                          privateKeyHex + " " + 
                          wif + " [GPU:" + std::to_string(gpuId) + "]";
        
        std::ofstream outFile(config_.search.outputFile, std::ios::app);
        if (outFile.is_open()) {
            outFile << line << std::endl;
            outFile.close();
        }
        
        // Create match result
        bitrecover::MatchResult match;
        match.address = result.address;
        match.privateKeyHex = privateKeyHex;
        match.wif = wif;
        match.gpuId = gpuId;
        match.timestamp = emailNotifier_->getCurrentTimestamp();
        
        // Send email notification
        emailNotifier_->sendMatchNotification(match);
        
        // Update display
        statusDisplay_->showMatch(match);
        
        Logger::log(LogLevel::Info, 
            "MATCH FOUND on GPU " + std::to_string(gpuId) + ": " + result.address);
    });
    
    // Status callback - update display
    gpuManager_->setStatusCallback([this](const bitrecover::GPUStats& stats) {
        // This will be called periodically
        // The display update will happen in the main loop
    });
}

void BitrecoverEngine::sendStartupNotification() {
    // Check if emails are enabled AND startup emails specifically are enabled
    if (!config_.email.enabled || !config_.email.sendStartupEmail) {
        return;
    }
    
    std::vector<bitrecover::GPUInfo> gpus;
    for (const auto& gpu : systemInfo_.gpus) {
        gpus.push_back(gpu);
    }
    
    emailNotifier_->sendStartupNotification(systemInfo_, gpus);
}

int BitrecoverEngine::run() {
    if (!initialized_) {
        Logger::log(LogLevel::Error, "Engine not initialized");
        return 1;
    }
    
    Logger::log(LogLevel::Info, "Starting parallel GPU search...");
    
    // Start status update loop
    std::thread statusThread([this]() {
        while (gpuManager_->isAnyRunning()) {
            auto stats = gpuManager_->getStats();
            statusDisplay_->update(stats);
            
            std::this_thread::sleep_for(
                std::chrono::milliseconds(config_.display.updateIntervalMs)
            );
        }
    });
    
    // Start GPU search
    gpuManager_->startParallelSearch(config_.search);
    
    // Wait for status thread
    if (statusThread.joinable()) {
        statusThread.join();
    }
    
    Logger::log(LogLevel::Info, "Search completed");
    
    return 0;
}

void BitrecoverEngine::stop() {
    if (gpuManager_) {
        gpuManager_->stopAll();
    }
    initialized_ = false;
}

