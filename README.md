## 概述
设备配置监控服务 - 用于监控和管理设备配置的Windows服务。

## 版本
1.0.1

## 构建要求
- Visual Studio 2022 或更高版本
- Windows SDK 10.0+
- C++17 编译器

## 构建步骤
1. 使用Visual Studio打开 `DeviceConfigMonitorService.vcxproj`
2. 选择 Release 或 Debug 配置
3. 构建解决方案

## 使用方法
### 控制台模式
```bash
D:/workspace/training/DeviceConfigMonitorService/x64/Debug/DeviceConfigMonitorService.exe --console
```

### 控制台模式行为说明
- 启动后自动从 `C:\ProgramData\NanningTraining\DeviceConfigMonitorService\config.json` 加载配置（不存在则自动生成默认配置）
- 配置中 `LogPath` 指定的目录会自动创建
- 心跳日志按 `HeartbeatIntervalSeconds` 周期写入该目录下的 `service-YYYY-MM-DD.log`
- 如果 `EnableHeartbeat=false`，则不输出周期心跳
- 如果 `HeartbeatIntervalSeconds<=0`，则使用默认值 5 秒
- 按 `Ctrl+C` 优雅退出（先停止心跳线程，再 flush 日志）

##############################################################################################################

## Overview
Device Configuration Monitoring Service – a Windows service for monitoring and managing device configurations.

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
