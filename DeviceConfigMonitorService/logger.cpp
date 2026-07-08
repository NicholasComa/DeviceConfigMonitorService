//============================================================================
// DeviceConfigMonitorService - Logger implementation
//============================================================================

#include "logger.h"

#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

//============================================================================
// Static member definitions
//============================================================================
std::string   Logger::s_logDir;
std::string   Logger::s_currentFile;
std::ofstream Logger::s_ofs;
std::mutex    Logger::s_mutex;
bool          Logger::s_initialized = false;

//============================================================================
// Internal helper: create directory recursively
// (Uses the same fix as config.cpp: skip bare drive letters)
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
            // Skip bare drive letters "C:" / "D:"
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

// Level -> string
const char* LevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
        default:              return "?";
    }
}

// Get current timestamp: [YYYY-MM-DD HH:MM:SS.mmm]
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

// Generate today's log file name: service-YYYY-MM-DD.log
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

// Cross-day detection: if the file name differs from today's expected name, reopen the file
// Note: This cannot access Logger private members from inside an anonymous namespace,
// so it is implemented as a private static method (Logger::RotateIfNeededInternal below)

} // namespace

//============================================================================
// Logger public API
//============================================================================
bool Logger::Init(const std::string& logDir) {
    std::lock_guard<std::mutex> lock(s_mutex);

    s_logDir = logDir;

    // 1) Ensure log directory exists
    if (!EnsureDirectory(logDir)) {
        std::cerr << "[Logger] Failed to create log directory: " << logDir << std::endl;
        return false;
    }

    // 2) Open today's log file
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

// Internal: reopen file on day rollover (private static method of Logger)
void Logger::RotateIfNeededInternal() {
    std::string expected = MakeLogFileName();
    if (expected != s_currentFile) {
        if (s_ofs.is_open()) {
            s_ofs.close();
        }
        s_currentFile = expected;
        std::string fullPath = s_logDir + "\\" + expected;
        s_ofs.open(fullPath, std::ios::app);
        if (!s_ofs.is_open()) {
            std::cerr << "[Logger] Failed to open log file: " << fullPath << std::endl;
        }
    }
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
        // Not initialized; output to stderr only, no file write
        std::cerr << "[" << LevelToString(level) << "] " << message << std::endl;
        return;
    }

    // Cross-day detection: switch to a new file when day changes
    RotateIfNeededInternal();

    std::string line = "[" + CurrentTimestamp() + "] ["
                     + LevelToString(level) + "] " + message;

    // Write to file
    if (s_ofs.is_open()) {
        s_ofs << line << std::endl;
        s_ofs.flush();
    }

    // Also output to console (convenient during development)
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
