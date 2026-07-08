//============================================================================
// DeviceConfigMonitorService - Windows Service framework
//============================================================================
// Day 4 task: extend --console mode into a real Windows Service.
// - ServiceMain is the entry point Windows SCM calls after StartService()
// - ServiceCtrlHandler receives control codes (STOP / SHUTDOWN / ...)
// - RunServiceBody() holds the shared business logic (config + log + heartbeat)
//   used by both --console and service modes
//============================================================================

#pragma once

#include <atomic>

// Windows types needed for ServiceMain / ServiceCtrlHandler signatures.
// Always include on Windows; the project is Windows-only anyway.
#ifdef _WIN32
#include <windows.h>   // WINAPI, DWORD, LPWSTR, SERVICE_STATUS, ...
#endif

//============================================================================
// 公共 API
//============================================================================

// 业务主体（控制台模式 / 服务模式共用）
// 流程：LoadConfig → Logger::Init → HeartbeatWorker.Start → 主循环 → 优雅停止
// stopFlag 由调用方控制：控制台模式下是 Ctrl+C atomic，服务模式下是 CtrlHandler 触发
// 内部循环每 200ms 检查一次 stopFlag，最长退出延迟 200ms
void RunServiceBody(std::atomic<bool>& stopFlag);

// Windows Service 入口（被 SCM 调用，**不直接由 main 调用**）
// 注册 ServiceCtrlHandler + 启动主循环
// 注意：LPSERVICE_MAIN_FUNCTION 是 windows.h 定义的函数指针类型
//       函数签名必须与 LPSERVICE_MAIN_FUNCTION 完全匹配
void WINAPI ServiceMain(DWORD argc, LPWSTR* argv);

// 控制回调：响应 SERVICE_CONTROL_STOP / SERVICE_CONTROL_SHUTDOWN
void WINAPI ServiceCtrlHandler(DWORD ctrlCode);
