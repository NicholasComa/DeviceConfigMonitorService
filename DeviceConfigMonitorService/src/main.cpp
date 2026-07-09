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

// Console-mode-specific stop flag
static std::atomic<bool> g_consoleStopRequested{false};

// Ctrl+C signal handler: triggers graceful exit of the while loop
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

        // Register Ctrl+C handler
        std::signal(SIGINT, ConsoleSignalHandler);
        std::signal(SIGTERM, ConsoleSignalHandler);

        // Print config summary (console-mode feature)
        AppConfig previewConfig = LoadConfig();
        PrintConfigSummary(previewConfig);
        // Note: LoadConfig already reads config.json;
        //       RunServiceBody will read it again. That is fine (IO is lightweight).

        std::cout << "Service is running. Press Ctrl+C to stop." << std::endl;
        std::cout << std::endl;

        // Shared business body
        RunServiceBody(g_consoleStopRequested);

        std::cout << "Service stopped gracefully." << std::endl;
    }
    else {
        // Windows Service mode
        // SERVICE_TABLE_ENTRY maps the service name to the ServiceMain entry point
        // StartServiceCtrlDispatcher blocks until the service stops
        // Note: In newer Windows SDK (10.0.19041+), SERVICE_TABLE_ENTRY
        //       has A and W variants, selected by the UNICODE macro.
        //       Our ServiceMain signature uses the wide-char version (LPWSTR*),
        //       so we explicitly use SERVICE_TABLE_ENTRYW for consistency.
        SERVICE_TABLE_ENTRYW serviceTable[] = {
            // L"..." is const, but lpServiceName is LPWSTR (non-const)
            // SCM never modifies this string, so const_cast is safe
            { const_cast<LPWSTR>(L"DeviceConfigMonitorService"), (LPSERVICE_MAIN_FUNCTIONW)ServiceMain },
            { nullptr, nullptr }
        };

        if (!StartServiceCtrlDispatcherW(serviceTable)) {
            // Start failed: possible causes include SCM being unavailable
            // (e.g. running the exe directly instead of via services.msc)
            // Write to stderr instead of calling Logger because Logger is not initialized yet
            std::cerr << "StartServiceCtrlDispatcher failed. "
                      << "If you want to run in console mode, use --console flag." << std::endl;
            return 1;
        }
    }

    return 0;
}
