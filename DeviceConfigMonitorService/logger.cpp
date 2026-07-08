//============================================================================
// DeviceConfigMonitorService - Logger implementation
//============================================================================

#include "logger.h"

#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

//============================================================================
// 静态成员定义
//============================================================================
std::string   Logger::s_logDir;
std::string   Logger::s_currentFile;
std::ofstream Logger::s_ofs;
std::mutex    Logger::s_mutex;
bool          Logger::s_initialized = false;

//============================================================================
// 内部工具：递归创建目录（沿用 config.cpp 的修复版，跳过裸盘符）
//============================================================================
namespace {

bool EnsureDirectory(const std::string& dirPath) {
#ifdef _WIN32
    std::string pathCopy = dirPath;
    for (auto& ch : pathCopy) {
        if (ch == '/') ch = '\\';
    }

    std::string current;
    for (size_t i = 0; i < pathCopy.size(); ++i) {
        current += pathCopy[i];
        if (pathCopy[i] == '\\' || i == pathCopy.size() - 1) {
            if (current.back() == '\\') {
                current.pop_back();
            }
            // 跳过裸盘符 "C:" / "D:"
            bool isBareDrive = (current.size() == 2 && current[1] == ':');
            if (!isBareDrive && !current.empty()
                && !CreateDirectoryA(current.c_str(), nullptr)) {
                DWORD err = GetLastError();
                if (err != ERROR_ALREADY_EXISTS) {
                    return false;
                }
            }
            if (pathCopy[i] == '\\') {
                current += '\\';
            }
        }
    }
    return true;
#else
    std::string cmd = "mkdir -p \"" + dirPath + "\"";
    return system(cmd.c_str()) == 0;
#endif
}

// 等级 → 字符串
const char* LevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
    }
    return "?";
}

// 取当前时间戳：[YYYY-MM-DD HH:MM:SS.mmm]
std::string CurrentTimestamp() {
    using namespace std::chrono;
    auto now    = system_clock::now();
    auto t      = system_clock::to_time_t(now);
    auto millis = duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000;

    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setw(3) << std::setfill('0') << millis;
    return oss.str();
}

// 生成当天日志文件名：service-YYYY-MM-DD.log
std::string MakeLogFileName() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto t   = system_clock::to_time_t(now);
    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif

    std::ostringstream oss;
    oss << "service-" << std::put_time(&tm_buf, "%Y-%m-%d") << ".log";
    return oss.str();
}

// 跨日检测：如果文件名与今天的预期不一致，重新打开文件
void RotateIfNeeded() {
    std::string expected = MakeLogFileName();
    if (expected != Logger::GetCurrentLogFile()) {
        if (Logger::s_ofs.is_open()) {
            Logger::s_ofs.close();
        }
        Logger::s_currentFile = expected;
        std::string fullPath = Logger::s_logDir + "\\" + expected;
        Logger::s_ofs.open(fullPath, std::ios::app);
        if (!Logger::s_ofs.is_open()) {
            std::cerr << "[Logger] Failed to open log file: " << fullPath << std::endl;
        }
    }
}

} // namespace

//============================================================================
// Logger 公共 API
//============================================================================
bool Logger::Init(const std::string& logDir) {
    std::lock_guard<std::mutex> lock(s_mutex);

    s_logDir = logDir;

    // 1) 确保日志目录存在
    if (!EnsureDirectory(logDir)) {
        std::cerr << "[Logger] Failed to create log directory: " << logDir << std::endl;
        return false;
    }

    // 2) 打开当天的日志文件
    s_currentFile = MakeLogFileName();
    std::string fullPath = s_logDir + "\\" + s_currentFile;

    s_ofs.open(fullPath, std::ios::app);
    if (!s_ofs.is_open()) {
        std::cerr << "[Logger] Failed to open log file: " << fullPath << std::endl;
        return false;
    }

    s_initialized = true;
    std::cerr << "[Logger] Log file: " << fullPath << std::endl;
    return true;
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (s_ofs.is_open()) {
        s_ofs.flush();
        s_ofs.close();
    }
    s_initialized = false;
}

void Logger::Info(const std::string& message) {
    Write(LogLevel::Info, message);
}

void Logger::Warn(const std::string& message) {
    Write(LogLevel::Warn, message);
}

void Logger::Error(const std::string& message) {
    Write(LogLevel::Error, message);
}

void Logger::Write(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(s_mutex);

    if (!s_initialized) {
        // 没有 Init 过，只输出到 stderr，不写文件
        std::cerr << "[" << LevelToString(level) << "] " << message << std::endl;
        return;
    }

    // 跨日检测：跨天时切换到新文件
    RotateIfNeeded();

    std::string line = "[" + CurrentTimestamp() + "] ["
                     + LevelToString(level) + "] " + message;

    // 写文件
    if (s_ofs.is_open()) {
        s_ofs << line << std::endl;
        s_ofs.flush();
    }

    // 同时输出到控制台（开发期方便观察）
    if (level == LogLevel::Error) {
        std::cerr << line << std::endl;
    } else {
        std::cout << line << std::endl;
    }
}

bool Logger::IsInitialized() {
    return s_initialized;
}

const std::string& Logger::GetCurrentLogFile() {
    return s_currentFile;
}
