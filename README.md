
## Overview
Device Configuration Monitoring Service ¨C a Windows service for monitoring and managing device configurations.

## Version
1.0.0

## Build Requirements
Visual Studio 2022 or later
Windows SDK 10.0+
C++17 compiler

## Build Steps
Open DeviceConfigMonitorService.vcxproj using Visual Studio.
Select either the Release or Debug configuration.
Build the solution.
  
## Usage
### Console Mode
```bash
 D:/workspace/training/DeviceConfigMonitorService/x64/Debug/DeviceConfigMonitorService.exe --console
```
### Console Mode Behavior Description
- Upon startup, configuration is automatically loaded from `C:\ProgramData\NanningTraining\DeviceConfigMonitorService\config.json` (if the file does not exist, a default configuration is generated automatically).
- The directory specified by `LogPath` in the configuration is created automatically.
- Heartbeat logs are written periodically to `service-YYYY-MM-DD.log` under that directory at the interval specified by `HeartbeatIntervalSeconds`.
- If `EnableHeartbeat=false`, no periodic heartbeat is output.
- If `HeartbeatIntervalSeconds<=0`, the default value of 5 seconds is used.
- Press `Ctrl+C` for graceful exit (the heartbeat thread is stopped first, then logs are flushed).
