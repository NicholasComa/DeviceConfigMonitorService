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
// 全局状态（Windows Service 必须用全局变量保存 ServiceStatusHandle）
//============================================================================
namespace {

SERVICE_STATUS        g_serviceStatus        = {};
SERVICE_STATUS_HANDLE g_serviceStatusHandle  = nullptr;
std::atomic<bool>     g_serviceStopRequested{false};

} // namespace

//============================================================================
// RunServiceBody - 控制台模式和服务模式共用的业务主体
//============================================================================
// 流程（与原 main.cpp 的 --console 流程一致）：
//   1) LoadConfig
//   2) Logger::Init
//   3) HeartbeatWorker.Start
//   4) 主循环：每 200ms 检查 stopFlag
//   5) 优雅停止：worker.Stop() + Logger::Shutdown()
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
// ServiceMain - Windows SCM 调用的入口
//============================================================================
// 注意：ServiceMain 函数的签名必须完全匹配
//   void WINAPI ServiceMain(DWORD argc, LPWSTR* argv)
// 它由 SCM 异步调用，函数返回 ≠ 服务停止；
// 我们在内部用 ReportStatus + 业务循环保持服务存活。
//============================================================================
void WINAPI ServiceMain(DWORD /*argc*/, LPWSTR* /*argv*/) {
    // 1) 注册控制处理器
    g_serviceStatusHandle = RegisterServiceCtrlHandlerW(
        L"DeviceConfigMonitorService",
        ServiceCtrlHandler);

    if (g_serviceStatusHandle == nullptr) {
        // 注册失败：无法继续（没有 handle 就无法上报状态）
        return;
    }

    // 2) 初始化 SERVICE_STATUS
    g_serviceStatus.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
    g_serviceStatus.dwServiceSpecificExitCode = 0;
    g_serviceStatus.dwWin32ExitCode           = 0;
    g_serviceStatus.dwWaitHint                = 1000; // 1 秒

    // 3) 上报 SERVICE_START_PENDING
    g_serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_serviceStatus.dwControlsAccepted = 0;  // START_PENDING 阶段不接受控制
    SetServiceStatus(g_serviceStatusHandle, &g_serviceStatus);

    // 4) 启动业务线程
    // 注意：ServiceMain 本身是 SCM 调用的回调，不能阻塞；
    //       必须把业务逻辑放到子线程里跑，ServiceMain 自己尽快返回。
    std::thread serviceThread([&]() {
        // 子线程里上报 RUNNING
        g_serviceStatus.dwCurrentState    = SERVICE_RUNNING;
        g_serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP
                                           | SERVICE_ACCEPT_SHUTDOWN;
        SetServiceStatus(g_serviceStatusHandle, &g_serviceStatus);

        // 业务主体（阻塞，直到 stopFlag 被 CtrlHandler 置位）
        RunServiceBody(g_serviceStopRequested);

        // 业务循环退出后上报 SERVICE_STOPPED
        g_serviceStatus.dwCurrentState    = SERVICE_STOPPED;
        g_serviceStatus.dwControlsAccepted = 0;
        SetServiceStatus(g_serviceStatusHandle, &g_serviceStatus);
    });

    // 5) ServiceMain 立即返回
    //    业务逻辑在 serviceThread 里跑
    if (serviceThread.joinable()) {
        serviceThread.join();
    }
}

//============================================================================
// ServiceCtrlHandler - 接收控制码
//============================================================================
// 目前只处理 STOP 和 SHUTDOWN；其他控制码直接返回。
//============================================================================
void WINAPI ServiceCtrlHandler(DWORD ctrlCode) {
    switch (ctrlCode) {
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
            // 1) 先上报 STOP_PENDING
            g_serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            SetServiceStatus(g_serviceStatusHandle, &g_serviceStatus);

            // 2) 置位停止标志，业务循环会退出
            g_serviceStopRequested.store(true);
            break;

        default:
            // 其他控制码直接返回
            break;
    }
}
