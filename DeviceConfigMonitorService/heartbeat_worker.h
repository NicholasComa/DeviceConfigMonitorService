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
// HeartbeatWorker - 后台心跳线程
//============================================================================
class HeartbeatWorker {
public:
    HeartbeatWorker();
    ~HeartbeatWorker();

    // 启动：传入配置（拷贝一份避免外部生命周期问题）
    // 内部根据 EnableHeartbeat 决定是否真正开线程
    void Start(const AppConfig& config);

    // 停止：设置停止标志 + join 线程
    // 多次调用安全
    void Stop();

    // 是否正在运行
    bool IsRunning() const { return running_.load(); }

private:
    // 线程主循环
    void Loop();

    // 工具：把间隔规整为合法值（<=0 → 5）
    static int NormalizeInterval(int seconds);

    std::thread        thread_;
    std::atomic<bool>  running_;
    std::atomic<bool>  stopRequested_;
    AppConfig          config_;
};
