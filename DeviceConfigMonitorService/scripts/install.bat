@echo off
setlocal

set "SERVICE_NAME=DeviceConfigMonitorService"
set "DISPLAY_NAME=Device Config Monitor Service"
set "BIN_PATH=%~dp0..\..\x64\Debug\DeviceConfigMonitorService.exe"

REM Use absolute path to sc.exe so it works even when System32
REM is somehow not in PATH.
set "SC=C:\Windows\System32\sc.exe"

echo [install] Registering service...
echo   Service name : %SERVICE_NAME%
echo   Display name : %DISPLAY_NAME%
echo   Binary path  : %BIN_PATH%

if not exist "%BIN_PATH%" (
    echo [ERROR] Binary not found: %BIN_PATH%
    echo Please build the project in Visual Studio first.
    exit /b 1
)

if not exist "%SC%" (
    echo [ERROR] sc.exe not found at %SC%
    exit /b 1
)

REM Check for admin: try to write to a system directory
echo. > "%SystemRoot%\System32\dcms_install_test.tmp" 2>nul
if errorlevel 1 (
    echo [ERROR] This script must be run as Administrator.
    del "%SystemRoot%\System32\dcms_install_test.tmp" 2>nul
    exit /b 1
)
del "%SystemRoot%\System32\dcms_install_test.tmp" 2>nul

"%SC%" query %SERVICE_NAME% >nul 2>&1
if not errorlevel 1 (
    echo [WARN] Service already exists, deleting first...
    "%SC%" delete %SERVICE_NAME%
    timeout /t 2 /nobreak >nul
)

"%SC%" create %SERVICE_NAME% binPath= "%BIN_PATH%" DisplayName= "%DISPLAY_NAME%" start= auto
if errorlevel 1 (
    echo [ERROR] sc create failed.
    exit /b 1
)

echo [install] Service registered. Use start.bat to start.
endlocal
