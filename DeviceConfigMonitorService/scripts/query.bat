@echo off
REM ============================================================
REM query.bat - Query DeviceConfigMonitorService status
REM ============================================================
REM Run as Administrator.
REM ============================================================

setlocal

set "SERVICE_NAME=DeviceConfigMonitorService"

echo [query] Querying service: %SERVICE_NAME%
echo.

sc query %SERVICE_NAME%
if errorlevel 1 (
    echo [WARN] Service not installed.
    exit /b 0
)

endlocal
