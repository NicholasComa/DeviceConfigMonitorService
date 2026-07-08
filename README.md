# DeviceConfigMonitorService

## Overview

DeviceConfigMonitorService is a C++17 Windows service that monitors device configurations,
writes periodic heartbeat logs, and supports both console mode (for development/debugging)
and full Windows Service mode (for production deployment).

## Version

1.0.0

## Build Requirements

- Visual Studio 2022 or later
- Windows SDK 10.0+
- C++17 compiler

## Build Steps

1. Open `DeviceConfigMonitorService.sln` in Visual Studio 2022.
2. Select `Debug | x64` or `Release | x64`.
3. Build the solution (`Build > Build Solution`).

The output binary will be at `x64\<Configuration>\DeviceConfigMonitorService.exe`.

## Project Structure

| Module | Files | Role |
|---|---|---|
| Config | `config.h` / `config.cpp` | Load and save JSON configuration from `C:\ProgramData\NanningTraining\DeviceConfigMonitorService\config.json` |
| Logger | `logger.h` / `logger.cpp` | Thread-safe, daily-rotated log writer with Info / Warn / Error levels |
| Heartbeat | `heartbeat_worker.h` / `heartbeat_worker.cpp` | Background thread that writes periodic heartbeat entries via Logger |
| Service Framework | `service_main.h` / `service_main.cpp` | Windows SCM integration (ServiceMain, ServiceCtrlHandler, status reporting) |
| Main | `main.cpp` | Entry point: `--console` mode or Windows Service dispatch |
| Scripts | `scripts/*.bat` | Service management scripts (install, start, stop, query, uninstall) |

## Configuration

The configuration file is located at:

```
C:\ProgramData\NanningTraining\DeviceConfigMonitorService\config.json
```

If the file does not exist, a default configuration is generated automatically.

### Default Values

```json
{
  "DeviceId": "DEVICE-0001",
  "EnableHeartbeat": true,
  "HeartbeatIntervalSeconds": 30,
  "LogLevel": "INFO",
  "LogPath": "C:\\ProgramData\\NanningTraining\\DeviceConfigMonitorService\\logs",
  "ServiceName": "DeviceConfigMonitorService"
}
```

- `HeartbeatIntervalSeconds` ¡X a value ? 0 defaults to 5 seconds.
- `LogPath` ¡X the directory is created automatically if it does not exist.
- If the JSON is malformed, the program catches exceptions and falls back to defaults
  without crashing.

## Usage

### Console Mode (Development / Debugging)

Run with the `--console` flag to execute in the foreground with console output:

```bash
DeviceConfigMonitorService.exe --console
```

Behavior:
- Loads configuration and prints a summary.
- Initializes the logger and starts the heartbeat worker.
- Press `Ctrl+C` for graceful shutdown (heartbeat thread is stopped first,
  then logs are flushed).

### Windows Service Mode (Production)

Run without flags to register as a Windows Service managed by SCM:

#### Register and Start

```bash
scripts\install.bat      # Register the service (Run as Administrator)
scripts\start.bat        # Start the service
```

#### Manage

```bash
scripts\query.bat        # Check service status (STATE, PID, controls accepted)
scripts\stop.bat         # Stop the service gracefully
scripts\uninstall.bat    # Remove from SCM (stops first if running)
```

All scripts use `%~dp0`-relative paths and require Administrator privileges.

The service responds to `SERVICE_CONTROL_STOP` and `SERVICE_CONTROL_SHUTDOWN`
control codes. Status transitions:

```
START_PENDING ¡÷ RUNNING ¡÷ STOP_PENDING ¡÷ STOPPED
```

On stop, the heartbeat thread is cleanly joined and logs are flushed before
the process exits.

## Logs

Log files are written to the directory specified by `LogPath` in the
configuration (default: `C:\ProgramData\NanningTraining\DeviceConfigMonitorService\logs`).

File format: `service-YYYY-MM-DD.log`

Each line follows:

```
[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] message
```

The log file automatically rotates at midnight (cross-day detection on
every write). The logger writes to both the file and the console during
development.

## Help

```bash
DeviceConfigMonitorService.exe --help
```

Displays usage information for both console and service modes.
