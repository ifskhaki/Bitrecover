#include "ConfigManager.h"
#include "bitrecover/constants.h"
#include <fstream>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <climits>
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif
#endif

ConfigManager::ConfigManager() {
    // Set defaults
    config_.email.enabled = false;
    config_.email.sendStartupEmail = false;  // NEW: Default to no startup emails
    config_.email.sendMatchEmail = true;     // NEW: Default to send match emails
    config_.email.smtpServer = bitrecover::DEFAULT_SMTP_SERVER;
    config_.email.smtpPort = bitrecover::DEFAULT_SMTP_PORT;
    
    config_.gpu.useAllGPUs = true;
    config_.gpu.autoOptimize = true;
    config_.gpu.threadsPerBlock = bitrecover::DEFAULT_THREADS_PER_BLOCK;
    config_.gpu.blocks = bitrecover::DEFAULT_BLOCKS;
    config_.gpu.pointsPerThread = bitrecover::DEFAULT_POINTS_PER_THREAD;
    
    config_.search.targetsFile = bitrecover::DEFAULT_TARGETS_FILE;
    config_.search.outputFile = bitrecover::DEFAULT_OUTPUT_FILE;
    config_.search.compression = bitrecover::DEFAULT_COMPRESSION;
    config_.search.random256 = bitrecover::DEFAULT_RANDOM256;
    config_.search.statusIntervalMs = bitrecover::DEFAULT_UPDATE_INTERVAL_MS;
    
    config_.display.realTime = bitrecover::DEFAULT_REAL_TIME;
    config_.display.updateIntervalMs = bitrecover::DEFAULT_UPDATE_INTERVAL_MS;
    config_.display.showGpuDetails = true;
    config_.display.clearScreen = true;
}

bool ConfigManager::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Simple JSON-like parser (basic key-value extraction)
    std::string line;
    while (std::getline(file, line)) {
        // Remove whitespace
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#' || line[0] == '/') {
            continue;
        }
        
        // Extract key-value pairs (simplified)
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // Remove trailing comma (JSON format)
            if (!value.empty() && value.back() == ',') {
                value = value.substr(0, value.length() - 1);
            }
            
            // Remove quotes
            if (value.length() >= 2 && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
            }
            
            parseConfigKey(key, value);
        }
    }
    
    return true;
}

void ConfigManager::parseConfigKey(const std::string& key, const std::string& value) {
    if (key == "email.enabled" || key == "\"enabled\"") {
        config_.email.enabled = (value == "true" || value == "1");
    } else if (key.find("send_startup_email") != std::string::npos) {
        config_.email.sendStartupEmail = (value == "true" || value == "1");
    } else if (key.find("send_match_email") != std::string::npos) {
        config_.email.sendMatchEmail = (value == "true" || value == "1");
    } else if (key.find("smtp_server") != std::string::npos) {
        config_.email.smtpServer = value;
    } else if (key.find("smtp_port") != std::string::npos) {
        config_.email.smtpPort = std::stoi(value);
    } else if (key.find("username") != std::string::npos && key.find("email") != std::string::npos) {
        config_.email.username = value;
    } else if (key.find("password") != std::string::npos && key.find("email") != std::string::npos) {
        config_.email.password = value;
    } else if (key.find("use_all_gpus") != std::string::npos) {
        config_.gpu.useAllGPUs = (value == "true" || value == "1");
    } else if (key.find("threads_per_block") != std::string::npos) {
        config_.gpu.threadsPerBlock = std::stoi(value);
    } else if (key.find("points_per_thread") != std::string::npos) {
        config_.gpu.pointsPerThread = std::stoi(value);
    } else if (key.find("targets_file") != std::string::npos) {
        config_.search.targetsFile = value;
    } else if (key.find("output_file") != std::string::npos) {
        config_.search.outputFile = value;
    } else if (key.find("compression") != std::string::npos) {
        config_.search.compression = value;
    } else if (key.find("random256") != std::string::npos) {
        config_.search.random256 = (value == "true" || value == "1");
    }
}

bitrecover::Config ConfigManager::getConfig() const {
    return config_;
}

void ConfigManager::setConfig(const bitrecover::Config& config) {
    config_ = config;
}

std::string ConfigManager::getHostname() const {
#ifdef _WIN32
    char hostname[256];
    DWORD size = sizeof(hostname);
    if (GetComputerNameA(hostname, &size)) {
        return std::string(hostname);
    }
    return "unknown";
#else
    char hostname[HOST_NAME_MAX];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        return std::string(hostname);
    }
    return "unknown";
#endif
}

