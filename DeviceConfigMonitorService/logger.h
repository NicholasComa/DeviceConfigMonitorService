//============================================================================
// DeviceConfigMonitorService - Logger module
//============================================================================
// Day 3 task: log writer with Info/Warn/Error levels.
// - Log file path is taken from AppConfig.LogPath
// - Log directory is created automatically if it doesn't exist
// - Each line: [YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] message
// - Thread-safe: a mutex guards the underlying ofstream
//============================================================================

#pragma once

#include <string>
#include <fstream>
#include <mutex>

//============================================================================
// LogLevel - 日志等级
//============================================================================
enum class LogLevel {
    Info,
    Warn,
    Error,
};

//============================================================================
// Logger - 单例风格的日志写入器
//============================================================================
// 设计：
//   - 通过 Init(logPath) 指定日志目录；日志文件名固定为 service-<日期>.log
//     例如 service-2026-07-08.log，每天一个文件，避免单文件过大
//   - Info/Warn/Error 三个公开方法，参数与 std::cout 一致（流式 API）
//   - 文件写入 + 控制台输出双重输出（开发期方便观察）
//   - 线程安全：所有写入操作由 mutex 串行化
//============================================================================
class Logger {
public:
    // 初始化日志目录（路径来自 AppConfig.LogPath）
    // 返回 true = 成功；返回 false = 目录创建失败（写入会静默跳过）
    static bool Init(const std::string& logDir);

    // 显式关闭（程序退出时调用，确保缓冲区 flush）
    static void Shutdown();

    // 三个等级的日志写入
    static void Info(const std::string& message);
    static void Warn(const std::string& message);
    static void Error(const std::string& message);

    // 内部实现：等级字符串 + 消息写入
    static void Write(LogLevel level, const std::string& message);

    // 查询状态（自检用）
    static bool IsInitialized();
    static const std::string& GetCurrentLogFile();

private:
    static std::string  s_logDir;
    static std::string  s_currentFile;
    static std::ofstream s_ofs;
    static std::mutex   s_mutex;
    static bool         s_initialized;
};
