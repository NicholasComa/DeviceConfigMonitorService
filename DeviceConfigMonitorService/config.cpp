#include "config.h"

#include <fstream>
#include <iostream>
#include <iomanip>

// Windows directory creation
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace {

// 创建配置文件所在目录（递归创建多级目录）
bool EnsureConfigDirectory(const std::string& filePath) {
    size_t lastSlash = filePath.find_last_of("\\/");
    if (lastSlash == std::string::npos) {
        return true; // no directory portion
    }
    std::string dirPath = filePath.substr(0, lastSlash);

#ifdef _WIN32
    // Windows: recursively create directories
    std::string pathCopy = dirPath;
    for (auto& ch : pathCopy) {
        if (ch == '/') ch = '\\';
    }

    std::string current;
    for (size_t i = 0; i < pathCopy.size(); ++i) {
        current += pathCopy[i];
        if (pathCopy[i] == '\\' || i == pathCopy.size() - 1) {
            if (current.back() == '\\') current.pop_back();
            if (!current.empty() && !CreateDirectoryA(current.c_str(), nullptr)) {
                if (GetLastError() != ERROR_ALREADY_EXISTS) {
                    return false;
                }
            }
            if (pathCopy[i] == '\\') current += '\\';
        }
    }
    return true;
#else
    // Linux/macOS: mkdir -p
    std::string cmd = "mkdir -p \"" + dirPath + "\"";
    return system(cmd.c_str()) == 0;
#endif
}

// 生成默认配置
AppConfig GetDefaultConfig() {
    AppConfig cfg;
    cfg.DeviceId     = "DEVICE-0001";
    cfg.ServiceName  = "DeviceConfigMonitorService";
    cfg.EnableHeartbeat = true;
    cfg.HeartbeatIntervalSeconds = 30;
    cfg.LogLevel     = "INFO";
    cfg.LogPath      = "C:\\ProgramData\\NanningTraining\\DeviceConfigMonitorService\\logs";
    return cfg;
}

} // anonymous namespace

AppConfig LoadConfig() {
    AppConfig config;

    std::ifstream ifs(CONFIG_FILE_PATH);
    if (!ifs.is_open()) {
        // config.json not found, generate default config
        std::cerr << "[WARN] Config file not found at: " << CONFIG_FILE_PATH << std::endl;
        std::cerr << "[INFO] Generating default configuration..." << std::endl;

        config = GetDefaultConfig();

        if (!SaveConfig(config)) {
            std::cerr << "[ERROR] Failed to save default configuration." << std::endl;
        } else {
            std::cerr << "[INFO] Default configuration saved to: " << CONFIG_FILE_PATH << std::endl;
        }
        return config;
    }

    try {
        std::string content((std::istreambuf_iterator<char>(ifs)),
                             std::istreambuf_iterator<char>());
        ifs.close();

        if (content.empty()) {
            std::cerr << "[WARN] Config file is empty, using defaults." << std::endl;
            config = GetDefaultConfig();
            return config;
        }

        json j = json::parse(content);
        config = j.get<AppConfig>();

        std::cerr << "[INFO] Configuration loaded from: " << CONFIG_FILE_PATH << std::endl;

    } catch (const json::parse_error& e) {
        std::cerr << "[ERROR] Failed to parse config file (JSON error): " << e.what() << std::endl;
        std::cerr << "[INFO] Falling back to default configuration." << std::endl;
        config = GetDefaultConfig();

    } catch (const json::out_of_range& e) {
        std::cerr << "[ERROR] Config field error: " << e.what() << std::endl;
        std::cerr << "[INFO] Falling back to default configuration." << std::endl;
        config = GetDefaultConfig();

    } catch (const json::type_error& e) {
        std::cerr << "[ERROR] Config type mismatch: " << e.what() << std::endl;
        std::cerr << "[INFO] Falling back to default configuration." << std::endl;
        config = GetDefaultConfig();

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Unexpected error loading config: " << e.what() << std::endl;
        std::cerr << "[INFO] Falling back to default configuration." << std::endl;
        config = GetDefaultConfig();
    }

    return config;
}

bool SaveConfig(const AppConfig& config) {
    try {
        if (!EnsureConfigDirectory(CONFIG_FILE_PATH)) {
            std::cerr << "[ERROR] Failed to create config directory." << std::endl;
            return false;
        }

        json j = config;
        std::string jsonStr = j.dump(2);

        std::ofstream ofs(CONFIG_FILE_PATH, std::ios::trunc);
        if (!ofs.is_open()) {
            std::cerr << "[ERROR] Failed to open config file for writing: "
                      << CONFIG_FILE_PATH << std::endl;
            return false;
        }

        ofs << jsonStr;
        ofs.close();

        std::cerr << "[INFO] Configuration saved to: " << CONFIG_FILE_PATH << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to save config: " << e.what() << std::endl;
        return false;
    }
}

void PrintConfigSummary(const AppConfig& config) {
    std::cout << std::endl;
    std::cout << "+------------------------------------------+" << std::endl;
    std::cout << "|        Configuration Summary             |" << std::endl;
    std::cout << "+------------------------------------------+" << std::endl;
    std::cout << "  DeviceId               : " << config.DeviceId << std::endl;
    std::cout << "  ServiceName            : " << config.ServiceName << std::endl;
    std::cout << "  EnableHeartbeat        : " << (config.EnableHeartbeat ? "true" : "false") << std::endl;
    std::cout << "  HeartbeatIntervalSeconds: " << config.HeartbeatIntervalSeconds << "s" << std::endl;
    std::cout << "  LogLevel               : " << config.LogLevel << std::endl;
    std::cout << "  LogPath                : " << config.LogPath << std::endl;
    std::cout << "+------------------------------------------+" << std::endl;
    std::cout << std::endl;
}
