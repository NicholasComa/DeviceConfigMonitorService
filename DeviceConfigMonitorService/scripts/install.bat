@echo off
REM ============================================================
REM install.bat - Register DeviceConfigMonitorService to SCM
REM ============================================================
REM Run as Administrator.
REM Service name : DeviceConfigMonitorService
REM Display name : Device Config Monitor Service
REM Binary path  : %~dp0..\x64\Debug\DeviceConfigMonitorService.exe
REM ============================================================

setlocal

set "SERVICE_NAME=DeviceConfigMonitorService"
set "DISPLAY_NAME=Device Config Monitor Service"
set "BIN_PATH=%~dp0..\x64\Debug\DeviceConfigMonitorService.exe"

echo [install] Registering service...
echo   Service name : %SERVICE_NAME%
echo   Display name : %DISPLAY_NAME%
echo   Binary path  : %BIN_PATH%

if not exist "%BIN_PATH%" (
    echo [ERROR] Binary not found: %BIN_PATH%
    echo Please build the project in Visual Studio first.
    exit /b 1
)

REM 注册前先尝试删除（避免重复注册报错）
sc query %SERVICE_NAME% >nul 2>&1
if not errorlevel 1 (
    echo [WARN] Service already exists, deleting first...
    sc delete %SERVICE_NAME%
    timeout /t 2 /nobreak >nul
)

REM 注册服务（注意 binPath= 后面必须有空格）
sc create %SERVICE_NAME% binPath= "%BIN_PATH%" DisplayName= "%DISPLAY_NAME%" start= auto

if errorlevel 1 (
    echo [ERROR] sc create failed. Did you run as Administrator?
    exit /b 1
)

echo [install] Service registered successfully.
echo [install] Use start.bat to start it.
endlocal
