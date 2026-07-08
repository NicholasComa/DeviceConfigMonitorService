@echo off
setlocal

set "SERVICE_NAME=DeviceConfigMonitorService"
set "SC=C:\Windows\System32\sc.exe"

echo [query] Querying service: %SERVICE_NAME%
echo.

"%SC%" query %SERVICE_NAME%
if errorlevel 1 (
    echo [WARN] Service not installed.
    exit /b 0
)

endlocal
