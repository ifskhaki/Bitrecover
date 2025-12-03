#include "BitrecoverEngine.h"
#include "Logger.h"
#include "CmdParse.h"
#include "util.h"
#include <iostream>
#include <string>

void printUsage() {
    std::cout << "Bitrecover - Multi-GPU Bitcoin Key Recovery Tool\n\n";
    std::cout << "Usage: bitrecover [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --config FILE          Configuration file (default: config/config.json)\n";
    std::cout << "  --targets FILE         Target addresses file (default: address.txt)\n";
    std::cout << "  --output FILE          Output file for matches (default: Success.txt)\n";
    std::cout << "  --random256            Use random 256-bit keys (pre-2012 wallets)\n";
    std::cout << "  --gpu ID               Use specific GPU ID (can specify multiple)\n";
    std::cout << "  --all-gpus             Use all available GPUs (default)\n";
    std::cout << "  --list-devices         List available GPU devices\n";
    std::cout << "  --help                 Show this help message\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  bitrecover --config config/config.json\n";
    std::cout << "  bitrecover --targets address.txt --output Success.txt --random256\n";
    std::cout << "  bitrecover --gpu 0 --gpu 1  # Use GPUs 0 and 1\n";
    std::cout << "\n";
}

void listDevices() {
    try {
        std::vector<DeviceManager::DeviceInfo> devices = DeviceManager::getDevices();
        
        std::cout << "Available GPU Devices:\n";
        std::cout << "====================\n\n";
        
        for (size_t i = 0; i < devices.size(); ++i) {
            const auto& dev = devices[i];
            std::cout << "ID: " << dev.id << "\n";
            std::cout << "  Name: " << dev.name << "\n";
            std::string typeStr = "Unknown";
            if (dev.type == DeviceManager::DeviceType::CUDA) {
                typeStr = "CUDA";
            } else if (dev.type == DeviceManager::DeviceType::OpenCL) {
                typeStr = "OpenCL";
            }
            std::cout << "  Type: " << typeStr << "\n";
            std::cout << "  Memory: " << (dev.memory / (1024 * 1024)) << " MB\n";
            std::cout << "  Compute Units: " << dev.computeUnits << "\n";
            std::cout << "\n";
        }
    } catch (DeviceManager::DeviceManagerException& ex) {
        std::cerr << "Error: " << ex.msg << std::endl;
    }
}

int main(int argc, char** argv) {
    // Parse command line arguments
    CmdParse parser;
    parser.add("", "--config", true);
    parser.add("", "--targets", true);
    parser.add("", "--output", true);
    parser.add("", "--random256", false);
    parser.add("", "--gpu", true);
    parser.add("", "--all-gpus", false);
    parser.add("", "--list-devices", false);
    parser.add("", "--help", false);
    
    try {
        parser.parse(argc, argv);
    } catch (std::string err) {
        std::cerr << "Error parsing arguments: " << err << std::endl;
        return 1;
    }
    
    std::vector<OptArg> args = parser.getArgs();
    
    // Check for help
    for (const auto& arg : args) {
        if (arg.equals("", "--help")) {
            printUsage();
            return 0;
        }
    }
    
    // Check for list-devices
    for (const auto& arg : args) {
        if (arg.equals("", "--list-devices")) {
            listDevices();
            return 0;
        }
    }
    
    // Create engine
    BitrecoverEngine engine;
    
    // Parse configuration file
    std::string configFile = "config/config.json";
    for (const auto& arg : args) {
        if (arg.equals("", "--config")) {
            configFile = arg.arg;
            break;
        }
    }
    
    // Initialize engine
    if (!engine.initialize(configFile)) {
        std::cerr << "Failed to initialize Bitrecover engine" << std::endl;
        return 1;
    }
    
    // Run
    int result = engine.run();
    
    return result;
}

