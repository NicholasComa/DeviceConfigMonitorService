//============================================================================
// DeviceConfigMonitorService - Heartbeat worker implementation
//============================================================================

#include "heartbeat_worker.h"
#include "logger.h"

#include <chrono>

//============================================================================
// 常量
//============================================================================
namespace {
constexpr int DEFAULT_HEARTBEAT_INTERVAL_SECONDS = 5;
}

//============================================================================
// HeartbeatWorker
//============================================================================
HeartbeatWorker::HeartbeatWorker()
    : running_(false)
    , stopRequested_(false) {
}

HeartbeatWorker::~HeartbeatWorker() {
    Stop();
}

int HeartbeatWorker::NormalizeInterval(int seconds) {
    if (seconds <= 0) {
        return DEFAULT_HEARTBEAT_INTERVAL_SECONDS;
    }
    return seconds;
}

void HeartbeatWorker::Start(const AppConfig& config) {
    if (running_.load()) {
        return; // 已经在跑
    }

    config_         = config;
    stopRequested_  = false;

    if (!config_.EnableHeartbeat) {
        // 配置要求关闭心跳：不启动线程
        Logger::Info("Heartbeat is disabled (EnableHeartbeat=false).");
        return;
    }

    int interval = NormalizeInterval(config_.HeartbeatIntervalSeconds);
    Logger::Info("Heartbeat worker starting. DeviceId="
                 + config_.DeviceId
                 + ", interval=" + std::to_string(interval) + "s");

    running_.store(true);
    thread_ = std::thread(&HeartbeatWorker::Loop, this);
}

void HeartbeatWorker::Stop() {
    if (!running_.load()) {
        return;
    }
    stopRequested_.store(true);
    if (thread_.joinable()) {
        thread_.join();
    }
    running_.store(false);
    Logger::Info("Heartbeat worker stopped.");
}

void HeartbeatWorker::Loop() {
    int interval = NormalizeInterval(config_.HeartbeatIntervalSeconds);

    // 第一次立刻发一条，然后按 interval 周期循环
    auto nextTime = std::chrono::steady_clock::now();

    while (!stopRequested_.load()) {
        // 写一条心跳
        Logger::Info("[Heartbeat] DeviceId=" + config_.DeviceId
                     + ", ServiceName=" + config_.ServiceName);

        // 推进到下一次时间点
        nextTime += std::chrono::seconds(interval);

        // 短间隔 sleep + 检查停止标志（避免大间隔下响应慢）
        // 每次最多睡 200ms，多次检查
        while (!stopRequested_.load()) {
            auto now = std::chrono::steady_clock::now();
            if (now >= nextTime) break;

            auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
                nextTime - now).count();
            auto sleepMs = (std::min)(static_cast<long long>(remaining), 200LL);
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
        }
    }
}
