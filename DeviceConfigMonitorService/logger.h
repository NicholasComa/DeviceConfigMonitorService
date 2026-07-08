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
// LogLevel - severity levels
//============================================================================
enum class LogLevel {
    Info,
    Warn,
    Error,
};

//============================================================================
// Logger - singleton-style log writer
//============================================================================
// Design:
//   - Init(logPath) specifies the log directory; log file name is fixed as service-<date>.log
//     e.g. service-2026-07-08.log, one file per day to prevent unbounded growth
//   - Info/Warn/Error are three public methods consistent with std::cout (stream-like API)
//   - Dual output: file write + console output (convenient during development)
//   - Thread-safe: all write operations serialized by a mutex
//============================================================================
class Logger {
public:
    // Initialize log directory (path from AppConfig.LogPath)
    // Returns true on success; false on directory creation failure (writes are silently skipped)
    static bool Init(const std::string& logDir);

    // Explicit shutdown (call on program exit to ensure buffer flush)
    static void Shutdown();

    // Three severity levels for log writing
    static void Info(const std::string& message);
    static void Warn(const std::string& message);
    static void Error(const std::string& message);

    // Internal: level string + message write
    static void Write(LogLevel level, const std::string& message);

    // Status queries (for self-check use)
    static bool IsInitialized();
    static const std::string& GetCurrentLogFile();

private:
    // Cross-day detection: switch to a new file when day changes
    static void RotateIfNeededInternal();

    static std::string  s_logDir;
    static std::string  s_currentFile;
    static std::ofstream s_ofs;
    static std::mutex   s_mutex;
    static bool         s_initialized;
};
