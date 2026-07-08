@echo off
setlocal

set "SERVICE_NAME=DeviceConfigMonitorService"

echo [start] Starting service: %SERVICE_NAME%

sc query %SERVICE_NAME% >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Service not installed. Run install.bat first.
    exit /b 1
)

sc start %SERVICE_NAME%
if errorlevel 1 (
    echo [ERROR] sc start failed. Run as Administrator?
    exit /b 1
)

echo [start] Service started. Use query.bat to check status.
endlocal
