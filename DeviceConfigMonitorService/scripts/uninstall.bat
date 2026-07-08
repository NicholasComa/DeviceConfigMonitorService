@echo off
setlocal

set "SERVICE_NAME=DeviceConfigMonitorService"

echo [uninstall] Removing service: %SERVICE_NAME%

sc query %SERVICE_NAME% >nul 2>&1
if errorlevel 1 (
    echo [WARN] Service not found.
    exit /b 0
)

sc stop %SERVICE_NAME% >nul 2>&1
timeout /t 3 /nobreak >nul

sc delete %SERVICE_NAME%
if errorlevel 1 (
    echo [ERROR] sc delete failed. Run as Administrator?
    exit /b 1
)

echo [uninstall] Service removed.
endlocal
