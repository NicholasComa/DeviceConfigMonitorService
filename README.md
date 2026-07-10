# DeviceConfigMonitorService

## 1. Project Overview

DeviceConfigMonitorService is a C++17 Windows service that monitors device
configurations and writes periodic heartbeat logs to a daily-rotated log file.

It supports two operating modes:

- **Console mode** (`--console`): foreground process for development and
  debugging. Loads configuration, prints a summary, then runs the heartbeat
  loop until `Ctrl+C`.
- **Windows Service mode** (no flag): registered as a Windows service via
  the `scripts/*.bat` control scripts, managed by the Windows Service
  Control Manager (SCM).

This project was implemented as the Week 1 deliverable of the
`NanningTraining / DeviceConfigMonitorService` plan.

## 2. Build Requirements

- Visual Studio 2022 (Community or higher)
- Windows SDK 10.0+
- C++17 compiler (MSVC v143 toolset)
- Windows 10/11 or Windows Server 2016+

## 3. Build Steps

1. Open `DeviceConfigMonitorService.sln` in Visual Studio 2022.
2. Select configuration `Debug | x64` (or `Release | x64`).
3. Build the solution (`Build > Build Solution` or `Ctrl+Shift+B`).

The output binary is produced at:

```
x64\Debug\DeviceConfigMonitorService.exe
```

> The project is compiled with `/std:c++17 /utf-8` on all four
> configurations. The `/utf-8` flag is required to correctly handle
> non-ASCII characters on Chinese Windows (code page 936).

## 4. Project Structure

```
DeviceConfigMonitorService/
+- DeviceConfigMonitorService.sln
+- README.md
+- README_CN.md
+- .gitignore
+- .gitattributes
+- x64/                          # 构建输出（已 gitignore）
+- DeviceConfigMonitorService/
   +- src/                       # 源码（C++ 实现文件）
   |  +- main.cpp                # 入口：--console 或 SCM 分发
   |  +- config.cpp              # AppConfig + JSON 加载/保存
   |  +- logger.cpp              # Logger
   |  +- heartbeat_worker.cpp    # HeartbeatWorker
   |  +- service_main.cpp        # ServiceMain + ServiceCtrlHandler
   +- config.h                   # AppConfig 头文件（与 src/config.cpp 配对）
   +- logger.h                   # 与 src/logger.cpp 配对
   +- heartbeat_worker.h         # 与 src/heartbeat_worker.cpp 配对
   +- service_main.h             # 与 src/service_main.cpp 配对
   +- config.example.json        # 配置样例
   +- DeviceConfigMonitorService.vcxproj
   +- DeviceConfigMonitorService.vcxproj.filters
   +- scripts/                   # 服务控制 .bat 文件
   |  +- install.bat
   |  +- uninstall.bat
   |  +- start.bat
   |  +- stop.bat
   |  +- query.bat
   +- docs/                      # 文档
   |  +- test_report.md
   |  +- week1_summary.md
   +- third_party/               # 第三方依赖库
      +- nlohmann/
         +- json.hpp             # nlohmann/json v3.12.0（仅头文件）
```

| Module | Source Files | Role |
|---|---|---|
| Config | `config.h`, `config.cpp` | AppConfig struct, JSON load/save, default generation, error handling |
| Logger | `logger.h`, `logger.cpp` | Daily-rotated log writer with thread safety |
| Heartbeat | `heartbeat_worker.h`, `heartbeat_worker.cpp` | Background thread writing periodic heartbeat entries |
| Service Framework | `service_main.h`, `service_main.cpp` | Windows SCM integration (ServiceMain + ServiceCtrlHandler) |
| Main | `main.cpp` | Entry point and mode dispatch |
| Scripts | `scripts/*.bat` | Service install/start/stop/query/uninstall |
| Third Party | `third_party/nlohmann/json.hpp` | JSON parsing (header-only) |

## 5. Configuration

The configuration file is read from and written to:

```
C:\ProgramData\NanningTraining\DeviceConfigMonitorService\config.json
```

If the file does not exist on startup, a default configuration is generated
and saved automatically.

### 5.1 Default Values

```json
{
  "DeviceId": "DEVICE-0001",
  "ServiceName": "DeviceConfigMonitorService",
  "EnableHeartbeat": true,
  "HeartbeatIntervalSeconds": 30,
  "LogLevel": "INFO",
  "LogPath": "C:\\ProgramData\\NanningTraining\\DeviceConfigMonitorService\\logs"
}
```

### 5.2 Field Reference

| Field | Type | Default | Notes |
|---|---|---|---|
| `DeviceId` | string | `DEVICE-0001` | Unique device identifier printed in heartbeat logs |
| `ServiceName` | string | `DeviceConfigMonitorService` | Service display name in SCM |
| `EnableHeartbeat` | bool | `true` | When `false`, no periodic heartbeat is written |
| `HeartbeatIntervalSeconds` | int | `30` | Period in seconds. A value <= 0 defaults to 5 seconds |
| `LogLevel` | string | `INFO` | Reserved for future use; current output always includes Info/Warn/Error |
| `LogPath` | string | `C:\ProgramData\...\logs` | Directory is created automatically if missing |

### 5.3 Malformed JSON

If the JSON cannot be parsed, the program:

1. Prints an error to stderr (e.g. `[ERROR] Failed to parse config file (JSON error): ...`).
2. Falls back to the default configuration.
3. Continues running without crashing.

## 6. Logging

Log files are written to the directory specified by `LogPath` (default
`C:\ProgramData\NanningTraining\DeviceConfigMonitorService\logs`).

### 6.1 File Format

```
service-YYYY-MM-DD.log
```

One file per day. Cross-day rollover is detected on every write.

### 6.2 Line Format

```
[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] message
```

Example:

```
[2026-07-08 14:00:00.123] [INFO] Service starting up...
[2026-07-08 14:00:00.124] [INFO] Heartbeat worker starting. DeviceId=DEVICE-0001, interval=10s
[2026-07-08 14:00:10.118] [INFO] [Heartbeat] DeviceId=DEVICE-0001, ServiceName=DeviceConfigMonitorService
```

Errors are written to `stderr`; Info/Warn are written to `stdout`.

## 7. Running the Program

### 7.1 Console Mode

```bat
cd x64\Debug
DeviceConfigMonitorService.exe --console
```

- Loads configuration and prints a summary.
- Initializes the Logger and starts the heartbeat worker.
- Press `Ctrl+C` for graceful shutdown (heartbeat thread is joined
  first, then logs are flushed).

### 7.2 Windows Service Mode

All control scripts must be run from an **Administrator** command prompt
because `sc.exe` requires `OpenSCManager` privileges.

```bat
:: Register the service
scripts\install.bat

:: Start the service
scripts\start.bat

:: Query status (STATE, PID, controls accepted)
scripts\query.bat

:: Stop the service (graceful)
scripts\stop.bat

:: Remove from SCM (stops first if running)
scripts\uninstall.bat
```

Service status transitions:

```
START_PENDING -> RUNNING -> STOP_PENDING -> STOPPED
```

The service responds to `SERVICE_CONTROL_STOP` and `SERVICE_CONTROL_SHUTDOWN`.
On stop, the heartbeat thread is joined and logs are flushed before the
process exits.

### 7.3 Help

```bat
DeviceConfigMonitorService.exe --help
```

## 8. FAQ / Troubleshooting

**Q: Git status shows .vs/, x64/ or *.user files even though .gitignore is in place.**
A: They may have been committed before .gitignore was added. Run
`git rm -r --cached .vs x64` and any tracked `*.user` file. Future builds
will not add them.

**Q: The build fails with "code page 936" / "cannot represent character" errors.**
A: This is the Chinese Windows GBK code page. The project compiles with
`/utf-8`; ensure your Visual Studio installation is 16.10+ and rebuild.
Do not edit source files in editors that do not save as UTF-8.

**Q: `install.bat` says "sc create failed" / "OpenSCManager failed 5".**
A: The script must be run from a command prompt launched as
Administrator. The script also checks for write access to
`%SystemRoot%\System32` to confirm privileges.

**Q: `start.bat` succeeds but the service exits immediately.**
A: Check `scripts\query.bat` for the exit reason. The most common
cause is a missing Visual C++ runtime on the target machine, or the
executable was moved/renamed after install. Re-run `install.bat`.

**Q: Logs are not being written.**
A: Verify the value of `LogPath` in `config.json`. Ensure the process
has write permission to that directory. In service mode, the LocalSystem
account is used, which can write to `C:\ProgramData\...` by default.

**Q: After changing `HeartbeatIntervalSeconds`, the heartbeat interval
does not update.**
A: The config is read only at startup. Stop and restart the service
(or the console process) to pick up changes.

**Q: How do I uninstall the service completely?**
A: Run `scripts\uninstall.bat` (it stops the service first if running).
This removes the service entry from the Windows registry.

**Q: How do I test the console mode after uninstalling?**
A: Just run `DeviceConfigMonitorService.exe --console` directly from
`x64\Debug\`. The Windows Service is not involved.

## 9. Notes

- This is a Week 1 training project. The current implementation is
  deliberately minimal: it focuses on clean architecture, thread safety,
  and a working SCM integration, not on production hardening (e.g. ACLs,
  event log integration, retry policy, etc.).
- All commit messages follow the department template:
  `TYPE: app: SUBJECT` where `TYPE` is one of `func / fix / conf / docs / build / style / refactor`.
