#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "bitrecover/types.h"
#include <string>

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager() = default;

    bool loadFromFile(const std::string& filename);
    bitrecover::Config getConfig() const;
    void setConfig(const bitrecover::Config& config);
    std::string getHostname() const;

private:
    bitrecover::Config config_;
    void parseConfigKey(const std::string& key, const std::string& value);
};

#endif // CONFIG_MANAGER_H

