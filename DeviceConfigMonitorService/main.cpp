//============================================================================
// DeviceConfigMonitorService - Main entry point
//============================================================================

#include <iostream>
#include <string>
#include <cstring>
#include <atomic>
#include <csignal>

#include "config.h"
#include "logger.h"
#include "heartbeat_worker.h"
#include "service_main.h"

#ifdef _WIN32
#include <windows.h>
#endif
const std::string PROGRAM_NAME = "DeviceConfigMonitorService";
const std::string VERSION = "1.0.0";

// 控制台模式专用停止标志
static std::atomic<bool> g_consoleStopRequested{false};

// Ctrl+C 信号处理：让 while 循环体优雅退出
static void ConsoleSignalHandler(int /*signum*/) {
    g_consoleStopRequested.store(true);
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
    std::cout << "  (no flag)    Run as Windows Service (registered via scripts/*.bat)" << std::endl;
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

        // 注册 Ctrl+C 处理
        std::signal(SIGINT, ConsoleSignalHandler);
        std::signal(SIGTERM, ConsoleSignalHandler);

        // 打印配置摘要（控制台模式特色）
        AppConfig previewConfig = LoadConfig();
        PrintConfigSummary(previewConfig);
        // 注意：LoadConfig 之后 config.json 已经被读取，
        //       RunServiceBody 里会再读一次。性能上没问题（IO 很轻）。

        std::cout << "Service is running. Press Ctrl+C to stop." << std::endl;
        std::cout << std::endl;

        // 共用业务主体
        RunServiceBody(g_consoleStopRequested);

        std::cout << "Service stopped gracefully." << std::endl;
    }
    else {
        // Windows Service 模式
        // SERVICE_TABLE_ENTRY 把服务名映射到 ServiceMain 入口
        // StartServiceCtrlDispatcher 会一直阻塞直到服务停止
        // 注意：在新版 Windows SDK（10.0.19041+）中，SERVICE_TABLE_ENTRY
        //       是 A/W 两个版本，默认根据 UNICODE 宏选择。
        //       我们的 ServiceMain 签名是宽字符版（LPWSTR*），
        //       所以显式使用 SERVICE_TABLE_ENTRYW 让两边一致。
        SERVICE_TABLE_ENTRYW serviceTable[] = {
            { L"DeviceConfigMonitorService", (LPSERVICE_MAIN_FUNCTIONW)ServiceMain },
            { nullptr, nullptr }
        };

        if (!StartServiceCtrlDispatcherW(serviceTable)) {
            // 启动失败：可能是 SCM 不可用（比如直接双击 exe 而不是用 services.msc）
            // 这里写 stderr 而不是调 Logger，因为 Logger 还未初始化
            std::cerr << "StartServiceCtrlDispatcher failed. "
                      << "If you want to run in console mode, use --console flag." << std::endl;
            return 1;
        }
    }

    return 0;
}
