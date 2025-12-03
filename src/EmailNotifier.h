#ifndef EMAIL_NOTIFIER_H
#define EMAIL_NOTIFIER_H

#include "bitrecover/types.h"
#include <string>
#include <vector>

class EmailNotifier {
public:
    EmailNotifier(const bitrecover::Config::EmailConfig& config);
    ~EmailNotifier() = default;

    void sendStartupNotification(const bitrecover::SystemInfo& sysInfo,
                                 const std::vector<bitrecover::GPUInfo>& gpus);
    void sendMatchNotification(const bitrecover::MatchResult& match);
    std::string getCurrentTimestamp() const;

private:
    bitrecover::Config::EmailConfig config_;
    void executeScript(const std::string& command);
};

#endif // EMAIL_NOTIFIER_H

