@echo off
setlocal

set "SERVICE_NAME=DeviceConfigMonitorService"
set "SC=C:\Windows\System32\sc.exe"

echo [stop] Stopping service: %SERVICE_NAME%

"%SC%" query %SERVICE_NAME% >nul 2>&1
if errorlevel 1 (
    echo [WARN] Service not installed.
    exit /b 0
)

"%SC%" stop %SERVICE_NAME%
if errorlevel 1 (
    echo [ERROR] sc stop failed. Run as Administrator?
    exit /b 1
)

echo [stop] Stop signal sent. Use query.bat to confirm.
endlocal
