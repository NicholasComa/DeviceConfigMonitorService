@echo off
REM ============================================================
REM uninstall.bat - Remove DeviceConfigMonitorService from SCM
REM ============================================================
REM Run as Administrator.
REM ============================================================

setlocal

set "SERVICE_NAME=DeviceConfigMonitorService"

echo [uninstall] Removing service: %SERVICE_NAME%

sc query %SERVICE_NAME% >nul 2>&1
if errorlevel 1 (
    echo [WARN] Service not found, nothing to do.
    exit /b 0
)

REM 先停服务
sc stop %SERVICE_NAME%
timeout /t 3 /nobreak >nul

sc delete %SERVICE_NAME%
if errorlevel 1 (
    echo [ERROR] sc delete failed. Did you run as Administrator?
    exit /b 1
)

echo [uninstall] Service removed successfully.
endlocal
