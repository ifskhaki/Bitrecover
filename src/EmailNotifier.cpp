#include "EmailNotifier.h"
#include "bitrecover/types.h"
#include "Logger.h"
#include "util.h"
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

EmailNotifier::EmailNotifier(const bitrecover::Config::EmailConfig& config)
    : config_(config) {
}

void EmailNotifier::sendStartupNotification(const bitrecover::SystemInfo& sysInfo,
                                           const std::vector<bitrecover::GPUInfo>& gpus) {
    // Check if emails are enabled AND startup emails specifically are enabled
    if (!config_.enabled || !config_.sendStartupEmail) {
        return;
    }
    
    std::ostringstream cmd;
#ifdef _WIN32
    cmd << "python scripts\\startup_notify.py";
#else
    cmd << "python3 scripts/startup_notify.py";
#endif
    
    // Pass system info as arguments
    cmd << " \"" << sysInfo.hostname << "\"";
    cmd << " \"" << sysInfo.os << "\"";
    cmd << " " << gpus.size();
    
    for (const auto& gpu : gpus) {
        cmd << " \"" << gpu.name << "\"";
        cmd << " " << (gpu.memory / (1024 * 1024)); // MB
    }
    
    Logger::log(LogLevel::Info, "Sending startup email notification...");
    executeScript(cmd.str());
}

void EmailNotifier::sendMatchNotification(const bitrecover::MatchResult& match) {
    // Check if emails are enabled AND match emails specifically are enabled
    if (!config_.enabled || !config_.sendMatchEmail) {
        return;
    }
    
    std::ostringstream cmd;
#ifdef _WIN32
    cmd << "python scripts\\send_email.py";
#else
    cmd << "python3 scripts/send_email.py";
#endif
    cmd << " \"" << match.address << "\"";
    cmd << " \"" << match.privateKeyHex << "\"";
    cmd << " \"" << match.wif << "\"";
    cmd << " " << match.gpuId;
    
    Logger::log(LogLevel::Info, "Sending match email notification...");
    executeScript(cmd.str());
}

void EmailNotifier::executeScript(const std::string& command) {
    int result = system(command.c_str());
    if (result != 0) {
        Logger::log(LogLevel::Warning, "Email script execution failed: " + command);
    }
}

std::string EmailNotifier::getCurrentTimestamp() const {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

