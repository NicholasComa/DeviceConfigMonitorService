//============================================================================
// DeviceConfigMonitorService - Windows Service implementation
//============================================================================

#include "service_main.h"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "config.h"
#include "logger.h"
#include "heartbeat_worker.h"

#ifdef _WIN32
#include <windows.h>
#endif

//============================================================================
// Global state (Windows Service must use global variables for ServiceStatusHandle)
//============================================================================
namespace {

SERVICE_STATUS        g_serviceStatus        = {};
SERVICE_STATUS_HANDLE g_serviceStatusHandle  = nullptr;
std::atomic<bool>     g_serviceStopRequested{false};

} // namespace

//============================================================================
// RunServiceBody - Shared business logic for console and service modes
//============================================================================
// Flow (same as --console in main.cpp):
//   1) LoadConfig
//   2) Logger::Init
//   3) HeartbeatWorker.Start
//   4) Main loop: check stopFlag every 200ms
//   5) Graceful shutdown: worker.Stop() + Logger::Shutdown()
//============================================================================
void RunServiceBody(std::atomic<bool>& stopFlag) {
    Logger::Info("Service starting up...");
    AppConfig config = LoadConfig();
    Logger::Info("Configuration loaded. DeviceId=" + config.DeviceId
                 + ", ServiceName=" + config.ServiceName
                 + ", LogPath=" + config.LogPath);

    if (!Logger::Init(config.LogPath)) {
        std::cerr << "[WARN] Logger init failed, console output only." << std::endl;
    }
    Logger::Info("Logger initialized. LogDir=" + config.LogPath);

    HeartbeatWorker worker;
    worker.Start(config);

    Logger::Info("Service entered main loop.");
    while (!stopFlag.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    Logger::Info("Stop signal received. Shutting down...");
    worker.Stop();
    Logger::Info("Service stopped.");
    Logger::Shutdown();
}

//============================================================================
// ServiceMain - Entry point called by Windows SCM
//============================================================================
// Note: The signature of ServiceMain must exactly match
//   void WINAPI ServiceMain(DWORD argc, LPWSTR* argv)
// It is called asynchronously by SCM; returning from this function does
// NOT mean the service has stopped. We keep the service alive by calling
// ReportStatus + running the business loop inside a worker thread.
//============================================================================
void WINAPI ServiceMain(DWORD /*argc*/, LPWSTR* /*argv*/) {
    // 1) Register the control handler
    g_serviceStatusHandle = RegisterServiceCtrlHandlerW(
        L"DeviceConfigMonitorService",
        ServiceCtrlHandler);

    if (g_serviceStatusHandle == nullptr) {
        // Registration failed: cannot continue (no handle means cannot report status)
        return;
    }

    // 2) Initialize SERVICE_STATUS
    g_serviceStatus.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
    g_serviceStatus.dwServiceSpecificExitCode = 0;
    g_serviceStatus.dwWin32ExitCode           = 0;
    g_serviceStatus.dwWaitHint                = 1000; // 1 second

    // 3) Report SERVICE_START_PENDING
    g_serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_serviceStatus.dwControlsAccepted = 0;  // No control accepted during START_PENDING
    SetServiceStatus(g_serviceStatusHandle, &g_serviceStatus);

    // 4) Start the business thread
    // Note: ServiceMain itself is a callback invoked by SCM and must not block;
    //       business logic must run in a child thread; ServiceMain returns promptly.
    std::thread serviceThread([&]() {
        // Report RUNNING from the child thread
        g_serviceStatus.dwCurrentState    = SERVICE_RUNNING;
        g_serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP
                                           | SERVICE_ACCEPT_SHUTDOWN;
        SetServiceStatus(g_serviceStatusHandle, &g_serviceStatus);

        // Business body (blocks until stopFlag is set by CtrlHandler)
        RunServiceBody(g_serviceStopRequested);

        // Report SERVICE_STOPPED after the business loop exits
        g_serviceStatus.dwCurrentState    = SERVICE_STOPPED;
        g_serviceStatus.dwControlsAccepted = 0;
        SetServiceStatus(g_serviceStatusHandle, &g_serviceStatus);
    });

    // 5) ServiceMain returns promptly
    //    Business logic runs in serviceThread
    if (serviceThread.joinable()) {
        serviceThread.join();
    }
}

//============================================================================
// ServiceCtrlHandler - Receives control codes
//============================================================================
// Currently handles STOP and SHUTDOWN only; other codes return immediately.
//============================================================================
void WINAPI ServiceCtrlHandler(DWORD ctrlCode) {
    switch (ctrlCode) {
        case SERVICE_CONTROL_STOP:
            // 1) Report STOP_PENDING first
            g_serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            SetServiceStatus(g_serviceStatusHandle, &g_serviceStatus);

            // 2) Set the stop flag so the business loop exits
            g_serviceStopRequested.store(true);
            break;

        case SERVICE_CONTROL_SHUTDOWN:
            // System shutdown: also exit gracefully
            g_serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            SetServiceStatus(g_serviceStatusHandle, &g_serviceStatus);
            g_serviceStopRequested.store(true);
            break;

        default:
            // Other control codes: return immediately
            break;
    }
}
