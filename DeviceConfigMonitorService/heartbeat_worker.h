//============================================================================
// DeviceConfigMonitorService - Heartbeat worker
//============================================================================
// Day 3 task: periodic heartbeat log writer.
// - Reads AppConfig.EnableHeartbeat and HeartbeatIntervalSeconds
// - When EnableHeartbeat is true, calls Logger::Info() every N seconds
// - When HeartbeatIntervalSeconds <= 0, uses default 5 seconds
// - Runs in a background std::thread; supports clean stop via atomic flag
//============================================================================

#pragma once

#include <atomic>
#include <thread>
#include <string>

#include "config.h"

//============================================================================
// HeartbeatWorker - background heartbeat thread
//============================================================================
class HeartbeatWorker {
public:
    HeartbeatWorker();
    ~HeartbeatWorker();

    // Start: takes a copy of the config (avoids external lifetime issues)
    // Internally decides whether to actually start the thread based on EnableHeartbeat
    void Start(const AppConfig& config);

    // Stop: set stop flag + join thread
    // Safe to call multiple times
    void Stop();

    // Whether the worker is currently running
    bool IsRunning() const { return running_.load(); }

private:
    // Thread main loop
    void Loop();

    // Helper: normalize interval to a valid value (<=0 → 5)
    static int NormalizeInterval(int seconds);

    std::thread        thread_;
    std::atomic<bool>  running_;
    std::atomic<bool>  stopRequested_;
    AppConfig          config_;
};
