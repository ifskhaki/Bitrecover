#ifndef BITRECOVER_CONSTANTS_H
#define BITRECOVER_CONSTANTS_H

#include <string>

namespace bitrecover {

// Default SMTP settings
const std::string DEFAULT_SMTP_SERVER = "smtp.gmail.com";
const int DEFAULT_SMTP_PORT = 587;

// Default GPU settings
const int DEFAULT_THREADS_PER_BLOCK = 256;
const int DEFAULT_POINTS_PER_THREAD = 32;
const int DEFAULT_BLOCKS = 0;  // Auto-detect

// Default search settings
const std::string DEFAULT_TARGETS_FILE = "address.txt";
const std::string DEFAULT_OUTPUT_FILE = "Success.txt";
const std::string DEFAULT_COMPRESSION = "UNCOMPRESSED";
const bool DEFAULT_RANDOM256 = true;

// Default display settings
const int DEFAULT_UPDATE_INTERVAL_MS = 1000;
const bool DEFAULT_REAL_TIME = true;

// File paths
const std::string CONFIG_FILE = "config/config.json";
const std::string STARTUP_SCRIPT = "scripts/startup_notify.py";
const std::string EMAIL_SCRIPT = "scripts/send_email.py";

} // namespace bitrecover

#endif // BITRECOVER_CONSTANTS_H

