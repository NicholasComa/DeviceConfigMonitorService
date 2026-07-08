@echo off
REM ============================================================
REM stop.bat - Stop DeviceConfigMonitorService
REM ============================================================
REM Run as Administrator.
REM ============================================================

setlocal

set "SERVICE_NAME=DeviceConfigMonitorService"

echo [stop] Stopping service: %SERVICE_NAME%

sc query %SERVICE_NAME% >nul 2>&1
if errorlevel 1 (
    echo [WARN] Service not installed.
    exit /b 0
)

sc stop %SERVICE_NAME%
if errorlevel 1 (
    echo [ERROR] sc stop failed. Did you run as Administrator?
    exit /b 1
)

echo [stop] Stop signal sent. Use query.bat to confirm.
endlocal
