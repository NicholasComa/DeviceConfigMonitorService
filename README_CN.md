# DeviceConfigMonitorService

## 1. 项目概述

DeviceConfigMonitorService 是一个 C++17 Windows 服务，用于监控设备配置并将周期性心跳日志写入每日轮转的日志文件。

它支持两种运行模式：

- **控制台模式（--console）**：前台进程，用于开发和调试。加载配置、打印摘要，然后运行心跳循环，直到按 Ctrl+C 退出。
- **Windows 服务模式（无标志）**：通过 `scripts/*.bat` 控制脚本注册为 Windows 服务，由 Windows 服务控制管理器（SCM）管理。

本项目是 NanningTraining / DeviceConfigMonitorService 计划的第 1 周交付成果。

---

## 2. 构建要求

- Visual Studio 2022（Community 或更高版本）
- Windows SDK 10.0+
- C++17 编译器（MSVC v143 工具集）
- Windows 10/11 或 Windows Server 2016+

---

## 3. 构建步骤

1. 在 Visual Studio 2022 中打开 `DeviceConfigMonitorService.sln`。
2. 选择配置 `Debug | x64`（或 `Release | x64`）。
3. 生成解决方案（“生成” > “生成解决方案”或按 `Ctrl+Shift+B`）。

输出二进制文件生成于：

```
x64\Debug\DeviceConfigMonitorService.exe
```

该项目在所有四种配置下均使用 `/std:c++17 /utf-8` 编译。`/utf-8` 标志是必需的，用于在中文 Windows（代码页 936）上正确处理非 ASCII 字符。

---

## 4. 项目结构

```
DeviceConfigMonitorService/
+- DeviceConfigMonitorService.sln
+- README.md
+- README_CN.md
+- .gitignore
+- .gitattributes
+- x64/                           # 构建输出（已忽略）
+- DeviceConfigMonitorService/
   +- main.cpp                    # 入口点：--console 或 SCM 分发
   +- config.h / config.cpp       # AppConfig + JSON 加载/保存
   +- logger.h / logger.cpp       # 线程安全日志记录器（Info/Warn/Error）
   +- heartbeat_worker.h / .cpp   # 后台心跳线程
   +- service_main.h / .cpp       # ServiceMain / ServiceCtrlHandler
   +- config.example.json         # 示例配置文件
   +- DeviceConfigMonitorService.vcxproj
   +- DeviceConfigMonitorService.vcxproj.filters
   +- scripts/
   |  +- install.bat              # sc create（以管理员身份运行）
   |  +- uninstall.bat            # sc delete
   |  +- start.bat                # sc start
   |  +- stop.bat                 # sc stop
   |  +- query.bat                # sc query
   +- third_party/
      +- nlohmann/
         +- json.hpp              # nlohmann/json v3.12.0（单头文件）
```

| 模块 | 源文件 | 作用 |
|------|--------|------|
| 配置 | config.h, config.cpp | AppConfig 结构体、JSON 加载/保存、默认生成、错误处理 |
| 日志 | logger.h, logger.cpp | 每日轮转的日志写入器，线程安全 |
| 心跳 | heartbeat_worker.h, heartbeat_worker.cpp | 后台线程，周期性写入心跳条目 |
| 服务框架 | service_main.h, service_main.cpp | Windows SCM 集成（ServiceMain + ServiceCtrlHandler） |
| 主入口 | main.cpp | 入口点和模式分发 |
| 脚本 | scripts/*.bat | 服务安装/启动/停止/查询/卸载 |
| 第三方 | third_party/nlohmann/json.hpp | JSON 解析（仅头文件） |

---

## 5. 配置

配置文件的读取和写入路径为：

```
C:\ProgramData\NanningTraining\DeviceConfigMonitorService\config.json
```

如果启动时该文件不存在，则会自动生成并保存默认配置。

### 5.1 默认值

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

### 5.2 字段参考

| 字段 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| DeviceId | string | DEVICE-0001 | 唯一设备标识符，打印在心跳日志中 |
| ServiceName | string | DeviceConfigMonitorService | SCM 中的服务显示名称 |
| EnableHeartbeat | bool | true | 为 false 时不写入周期性心跳 |
| HeartbeatIntervalSeconds | int | 30 | 周期（秒）。值 <= 0 时默认为 5 秒 |
| LogLevel | string | INFO | 保留供将来使用；当前输出始终包含 Info/Warn/Error |
| LogPath | string | C:\ProgramData\...\logs | 目录不存在时自动创建 |

### 5.3 格式错误的 JSON

如果 JSON 无法解析，程序会：

- 向 stderr 打印错误（例如 `[ERROR] Failed to parse config file (JSON error): ...`）
- 回退到默认配置
- 继续运行，不会崩溃

---

## 6. 日志

日志文件写入 `LogPath` 指定的目录（默认为 `C:\ProgramData\NanningTraining\DeviceConfigMonitorService\logs`）。

### 6.1 文件格式

```
service-YYYY-MM-DD.log
```
每天一个文件。每次写入时都会检测跨天轮转。

### 6.2 行格式

```
[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] message
```

示例：

```
[2026-07-08 14:00:00.123] [INFO] Service starting up...
[2026-07-08 14:00:00.124] [INFO] Heartbeat worker starting. DeviceId=DEVICE-0001, interval=10s
[2026-07-08 14:00:10.118] [INFO] [Heartbeat] DeviceId=DEVICE-0001, ServiceName=DeviceConfigMonitorService
```

错误写入 stderr；Info/Warn 写入 stdout。

---

## 7. 运行程序

### 7.1 控制台模式

```cmd
cd x64\Debug
DeviceConfigMonitorService.exe --console
```

- 加载配置并打印摘要
- 初始化日志记录器并启动心跳工作线程
- 按 Ctrl+C 优雅关闭（先加入心跳线程，然后刷新日志）

### 7.2 Windows 服务模式

所有控制脚本必须从**管理员命令提示符**运行，因为 `sc.exe` 需要 `OpenSCManager` 权限。

```cmd
:: 注册服务
scripts\install.bat

:: 启动服务
scripts\start.bat

:: 查询状态（STATE、PID、接受的控件）
scripts\query.bat

:: 停止服务（优雅）
scripts\stop.bat

:: 从 SCM 中移除（如正在运行则先停止）
scripts\uninstall.bat
```

服务状态转换：

```
START_PENDING -> RUNNING -> STOP_PENDING -> STOPPED
```

该服务响应 `SERVICE_CONTROL_STOP` 和 `SERVICE_CONTROL_SHUTDOWN`。停止时，心跳线程会被加入，日志会在进程退出前刷新。

### 7.3 帮助

```cmd
DeviceConfigMonitorService.exe --help
```

---

## 8. 常见问题 / 故障排除

**问：Git status 显示 `.vs/`、`x64/` 或 `.user` 文件，尽管 `.gitignore` 已存在。**  
答：这些文件可能在添加 `.gitignore` 之前已被提交。运行 `git rm -r --cached .vs x64` 以及任何已跟踪的 `*.user` 文件。后续构建不会再添加它们。

**问：构建失败，出现“code page 936”/“cannot represent character”错误。**  
答：这是中文 Windows GBK 代码页问题。该项目使用 `/utf-8` 编译；请确保 Visual Studio 版本为 16.10+ 并重新生成。不要在不以 UTF-8 保存的编辑器中编辑源文件。

**问：`install.bat` 提示“sc create failed”/“OpenSCManager failed 5”。**  
答：脚本必须从“以管理员身份运行”的命令提示符中执行。该脚本还会检查对 `%SystemRoot%\System32` 的写入权限以确认权限。

**问：`start.bat` 成功但服务立即退出。**  
答：使用 `scripts\query.bat` 检查退出原因。最常见的原因是目标机器缺少 Visual C++ 运行时，或者可执行文件在安装后被移动/重命名。重新运行 `install.bat`。

**问：日志未被写入。**  
答：检查 `config.json` 中 `LogPath` 的值。确保进程对该目录有写入权限。在服务模式下，使用的是 LocalSystem 帐户，默认可以写入 `C:\ProgramData\...`。

**问：更改 `HeartbeatIntervalSeconds` 后，心跳间隔没有更新。**  
答：配置仅在启动时读取。停止并重新启动服务（或控制台进程）以生效。

**问：如何完全卸载服务？**  
答：运行 `scripts\uninstall.bat`（如果服务正在运行，会先停止它）。这将从 Windows 注册表中移除服务条目。

**问：卸载后如何测试控制台模式？**  
答：直接从 `x64\Debug\` 运行 `DeviceConfigMonitorService.exe --console` 即可。不涉及 Windows 服务。

---

## 9. 说明

- 这是一个第 1 周培训项目。当前实现刻意保持简洁：专注于清晰的架构、线程安全和可用的 SCM 集成，而非生产环境加固（如 ACL、事件日志集成、重试策略等）。
- 所有提交信息遵循部门模板：`TYPE: app: SUBJECT`，其中 `TYPE` 为 `func / fix / conf / docs / build / style / refactor` 之一。