//============================================================================
// DeviceConfigMonitorService - Main entry point
//============================================================================

#include <iostream>
#include <string>
#include <cstring>
#include <atomic>
#include <csignal>
#include <thread>
#include <chrono>

#include "config.h"
#include "logger.h"
#include "heartbeat_worker.h"

const std::string PROGRAM_NAME = "DeviceConfigMonitorService";
const std::string VERSION = "1.0.0";

// 全局停止标志（信号处理函数置位）
static std::atomic<bool> g_stopRequested{false};

// Ctrl+C 信号处理：让 while 循环体优雅退出
static void SignalHandler(int /*signum*/) {
    g_stopRequested.store(true);
}

void printBanner() {
    std::cout << "========================================" << std::endl;
    std::cout << "  " << PROGRAM_NAME << std::endl;
    std::cout << "  Version: " << VERSION << std::endl;
    std::cout << "========================================" << std::endl;
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [--console]" << std::endl;
    std::cout << "  --console    Run in console mode (with config + heartbeat)" << std::endl;
    std::cout << "  --help       Show this help message" << std::endl;
}

int main(int argc, char* argv[]) {
    bool consoleMode = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--console") == 0) {
            consoleMode = true;
        }
        else if (strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        }
    }

    if (consoleMode) {
        printBanner();

        // 1) 读取配置
        Logger::Info("Service starting up...");
        AppConfig config = LoadConfig();
        Logger::Info("Configuration loaded. DeviceId=" + config.DeviceId
                     + ", ServiceName=" + config.ServiceName
                     + ", LogPath=" + config.LogPath);
        PrintConfigSummary(config);

        // 2) 初始化日志（路径来自 config.LogPath）
        if (!Logger::Init(config.LogPath)) {
            std::cerr << "[WARN] Logger init failed, console output only." << std::endl;
        }
        Logger::Info("Logger initialized. LogDir=" + config.LogPath);

        // 3) 注册 Ctrl+C 处理
        std::signal(SIGINT, SignalHandler);
        std::signal(SIGTERM, SignalHandler);

        // 4) 启动心跳 worker
        HeartbeatWorker worker;
        worker.Start(config);

        // 5) 主循环：等待 Ctrl+C
        std::cout << std::endl;
        std::cout << "Service is running. Press Ctrl+C to stop." << std::endl;
        Logger::Info("Service entered main loop. Press Ctrl+C to stop.");

        while (!g_stopRequested.load()) {
            // 短间隔 sleep，让循环有退出窗口
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        // 6) 优雅停止
        Logger::Info("Stop signal received. Shutting down...");
        worker.Stop();
        Logger::Info("Service stopped.");
        Logger::Shutdown();

        std::cout << "Service stopped gracefully." << std::endl;
    }
    else {
        std::cout << "Starting " << PROGRAM_NAME << " as Windows Service..." << std::endl;
        std::cout << "Use --console flag to run in console mode." << std::endl;
        // Windows Service 模式将在 Day 4 实现
    }

    return 0;
}
