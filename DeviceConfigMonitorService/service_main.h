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
// Public API
//============================================================================

// Shared business body (used by both console and service modes)
// Flow: LoadConfig -> Logger::Init -> HeartbeatWorker.Start -> main loop -> graceful shutdown
// stopFlag is controlled by the caller: Ctrl+C atomic in console mode, CtrlHandler in service mode
// Inner loop checks stopFlag every 200ms; worst-case exit latency is 200ms
void RunServiceBody(std::atomic<bool>& stopFlag);

// Windows Service entry point (called by SCM, **not directly from main**)
// Registers ServiceCtrlHandler + starts the main loop
// Note: LPSERVICE_MAIN_FUNCTION is a function-pointer type defined in windows.h
//       The function signature must exactly match LPSERVICE_MAIN_FUNCTION
void WINAPI ServiceMain(DWORD argc, LPWSTR* argv);

// Control callback: handles SERVICE_CONTROL_STOP / SERVICE_CONTROL_SHUTDOWN
void WINAPI ServiceCtrlHandler(DWORD ctrlCode);
