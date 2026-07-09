//============================================================================
// DeviceConfigMonitorService - Heartbeat worker implementation
//============================================================================

#include "heartbeat_worker.h"
#include "logger.h"

#include <chrono>

//============================================================================
// Constants
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
        return; // already running
    }

    config_         = config;
    stopRequested_  = false;

    if (!config_.EnableHeartbeat) {
        // Config disables heartbeat: do not start the thread
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

    // Emit one heartbeat immediately, then cycle on the interval
    auto nextTime = std::chrono::steady_clock::now();

    while (!stopRequested_.load()) {
        // Write a heartbeat entry
        Logger::Info("[Heartbeat] DeviceId=" + config_.DeviceId
                     + ", ServiceName=" + config_.ServiceName);

        // Advance to the next time point
        nextTime += std::chrono::seconds(interval);

        // Short-interval sleep + stop flag check (avoids slow exit on long intervals)
        // Sleep at most 200ms per iteration, checking frequently
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
