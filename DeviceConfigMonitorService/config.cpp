#include "config.h"

#include <fstream>
#include <iostream>
#include <iomanip>

// Windows 目录创建
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
        return true; // 无目录部分，直接返回
    }
    std::string dirPath = filePath.substr(0, lastSlash);

#ifdef _WIN32
    // Windows: 递归创建目录
    std::string pathCopy = dirPath;
    // 将 / 转为 \ 统一处理
    for (auto& ch : pathCopy) {
        if (ch == '/') ch = '\\';
    }

    // 逐级创建目录
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
        // config.json 不存在，生成默认配置
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
        // 读取文件内容
        std::string content((std::istreambuf_iterator<char>(ifs)),
                             std::istreambuf_iterator<char>());
        ifs.close();

        if (content.empty()) {
            // 文件为空，使用默认配置
            std::cerr << "[WARN] Config file is empty, using defaults." << std::endl;
            config = GetDefaultConfig();
            return config;
        }

        // 反序列化 JSON
        json j = json::parse(content);
        config = j.get<AppConfig>();

        std::cerr << "[INFO] Configuration loaded from: " << CONFIG_FILE_PATH << std::endl;

    } catch (const json::parse_error& e) {
        // JSON 格式错误，捕获异常，使用默认配置，程序不崩溃
        std::cerr << "[ERROR] Failed to parse config file (JSON error): " << e.what() << std::endl;
        std::cerr << "[INFO] Falling back to default configuration." << std::endl;
        config = GetDefaultConfig();

    } catch (const json::out_of_range& e) {
        // JSON 字段缺失异常
        std::cerr << "[ERROR] Config field error: " << e.what() << std::endl;
        std::cerr << "[INFO] Falling back to default configuration." << std::endl;
        config = GetDefaultConfig();

    } catch (const json::type_error& e) {
        // JSON 类型错误异常
        std::cerr << "[ERROR] Config type mismatch: " << e.what() << std::endl;
        std::cerr << "[INFO] Falling back to default configuration." << std::endl;
        config = GetDefaultConfig();

    } catch (const std::exception& e) {
        // 其他未知异常
        std::cerr << "[ERROR] Unexpected error loading config: " << e.what() << std::endl;
        std::cerr << "[INFO] Falling back to default configuration." << std::endl;
        config = GetDefaultConfig();
    }

    return config;
}

bool SaveConfig(const AppConfig& config) {
    try {
        // 确保目录存在
        if (!EnsureConfigDirectory(CONFIG_FILE_PATH)) {
            std::cerr << "[ERROR] Failed to create config directory." << std::endl;
            return false;
        }

        // 序列化为格式化的 JSON（缩进 2 空格）
        json j = config;
        std::string jsonStr = j.dump(2);

        // 写入文件
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
