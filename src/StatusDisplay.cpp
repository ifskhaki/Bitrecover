#include "StatusDisplay.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif

StatusDisplay::StatusDisplay() : firstUpdate_(true) {
}

void StatusDisplay::update(const std::vector<bitrecover::GPUStats>& stats) {
    if (firstUpdate_) {
        clear();
        printHeader();
        firstUpdate_ = false;
    }
    
    // Move cursor to start of GPU stats area
    std::cout << "\033[2J\033[H";  // Clear and move to top
    
    printHeader();
    
    std::cout << "\n";
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << " GPU Status\n";
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "\n";
    
    for (const auto& stat : stats) {
        printGPUStats(stat);
        std::cout << "\n";
    }
    
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout.flush();
}

void StatusDisplay::printHeader() {
    std::cout << "╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║          Bitrecover - Multi-GPU Bitcoin Key Finder          ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝\n";
}

void StatusDisplay::printGPUStats(const bitrecover::GPUStats& stats) {
    std::cout << "GPU " << stats.gpuId << ": " << stats.name << "\n";
    std::cout << "  Status: " << (stats.isRunning ? "RUNNING" : "STOPPED") << "\n";
    std::cout << "  Speed: " << std::fixed << std::setprecision(2) 
              << stats.speedMKeysPerSec << " MKeys/s\n";
    std::cout << "  Keys Processed: " << stats.keysProcessed << "\n";
    std::cout << "  Utilization: " << std::setprecision(1) 
              << stats.utilizationPercent << "%\n";
    if (!stats.status.empty()) {
        std::cout << "  " << stats.status << "\n";
    }
}

void StatusDisplay::showMatch(const bitrecover::MatchResult& match) {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    🎉 MATCH FOUND! 🎉                        ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝\n";
    std::cout << "Address: " << match.address << "\n";
    std::cout << "Private Key: " << match.privateKeyHex << "\n";
    std::cout << "WIF: " << match.wif << "\n";
    std::cout << "GPU: " << match.gpuId << "\n";
    std::cout << "Time: " << match.timestamp << "\n";
    std::cout << "\n";
    std::cout.flush();
}

void StatusDisplay::clear() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

