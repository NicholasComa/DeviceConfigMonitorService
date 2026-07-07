#pragma once

#include <string>
#include "third_party/nlohmann/json.hpp"

using json = nlohmann::json;

// 应用程序配置结构体
struct AppConfig {
    std::string DeviceId;                   // 设备唯一标识
    std::string ServiceName;                // 服务名称
    bool        EnableHeartbeat = true;     // 是否启用心跳
    int         HeartbeatIntervalSeconds = 30; // 心跳间隔（秒）
    std::string LogLevel = "INFO";          // 日志级别：DEBUG/INFO/WARN/ERROR
    std::string LogPath;                    // 日志文件路径

    // nlohmann/json 序列化/反序列化宏
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

// 配置文件路径常量
const std::string CONFIG_FILE_PATH =
    "C:\\ProgramData\\NanningTraining\\DeviceConfigMonitorService\\config.json";

// 加载配置：读取 config.json 并反序列化到 AppConfig
// config.json 不存在时生成默认配置并保存
// config.json 格式错误时捕获异常，返回默认配置，程序不崩溃
AppConfig LoadConfig();

// 保存配置：将 AppConfig 序列化保存为 config.json
// 自动创建目录（如不存在）
bool SaveConfig(const AppConfig& config);

// 打印配置摘要到控制台
void PrintConfigSummary(const AppConfig& config);
