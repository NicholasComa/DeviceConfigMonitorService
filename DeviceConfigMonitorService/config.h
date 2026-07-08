#pragma once

#include <string>
#include "third_party/nlohmann/json.hpp"

using json = nlohmann::json;

// Application configuration struct
struct AppConfig {
    std::string DeviceId;                       // Unique device identifier
    std::string ServiceName;                    // Service display name
    bool        EnableHeartbeat = true;         // Whether heartbeat is enabled
    int         HeartbeatIntervalSeconds = 30;  // Heartbeat interval in seconds
    std::string LogLevel = "INFO";              // Log level: DEBUG/INFO/WARN/ERROR
    std::string LogPath;                        // Log file directory path

    // nlohmann/json serialize/deserialize macro
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        AppConfig,
        DeviceId,
        ServiceName,
        EnableHeartbeat,
        HeartbeatIntervalSeconds,
        LogLevel,
        LogPath
    )
};

// Config file path constant
const std::string CONFIG_FILE_PATH =
    "C:\\ProgramData\\NanningTraining\\DeviceConfigMonitorService\\config.json";

// Load config: read config.json and deserialize to AppConfig
// Generates default config and saves it if config.json does not exist
// Catches exceptions on malformed JSON and returns default config without crashing
AppConfig LoadConfig();

// Save config: serialize AppConfig and write to config.json
// Auto-creates directory if it does not exist
bool SaveConfig(const AppConfig& config);

// Print config summary to console
void PrintConfigSummary(const AppConfig& config);
