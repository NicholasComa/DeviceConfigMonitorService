# Week 1 Summary - DeviceConfigMonitorService

**Author:** Week 1 trainee
**Branch:** `week5-cpp-service` (Week 1 of the NanningTraining programme)
**Date:** 2026-07-08
**Programme:** Nanning Software Group, 1st-week training plan

---

## 1. Project Overview

`DeviceConfigMonitorService` is a C++17 Windows background service that
monitors device configurations and writes periodic heartbeat logs to a
local log file. It was implemented as the Week 1 deliverable of the
Nanning software group training programme.

### 1.1 Purpose

- Provide a minimal but production-shaped reference for a C++ Windows
  Service, including: configuration management, structured logging,
  background heartbeat, and Windows Service Control Manager (SCM)
  integration.
- Exercise the full development loop on Windows: Git branching,
  Visual Studio C++ project, MSVC compiler, batch scripts, and
  deployment as a real Windows Service.

### 1.2 Operating Modes

The same binary supports two modes:

| Mode | Invocation | Use Case |
|---|---|---|
| Console mode | `DeviceConfigMonitorService.exe --console` | Development, debugging |
| Windows Service mode | `scripts\install.bat` + `scripts\start.bat` | Production deployment |

### 1.3 Week 1 Scope

| Day | Topic | Status |
|---|---|---|
| Day 1 | Git basics, commit template, VS C++ project init | Completed |
| Day 2 | AppConfig + JSON config load/save + error handling | Completed |
| Day 3 | Logger module + Heartbeat worker + console mode | Completed |
| Day 4 | Windows Service framework + control scripts | Completed |
| Day 5 | Repository cleanup, full test pass, documentation | Completed (this report) |

## 2. Git Commit Record

The branch `week5-cpp-service` contains 24 commits at the time of this
report. Below are the major commits grouped by purpose. The full log
can be viewed with `git log --oneline --graph`.

### 2.1 Day 1 - Project Initialization

| Commit | Message | Purpose |
|---|---|---|
| `1526fdc` | func: app: Add minimal entry program | First C++ entry, supports `--console` |
| `c453ee7` | docs: app: Add training project README | Initial README |
| `884a3cf` | docs: app: Supplement the training project README | README updates |

### 2.2 Day 2 - Configuration Model and JSON Parser

| Commit | Message | Purpose |
|---|---|---|
| `8037d13` | func: app: Add application config model | AppConfig struct + NLOHMANN_DEFINE_TYPE_INTRUSIVE |
| `4048db1` | func: app: Add JSON config parser | LoadConfig / SaveConfig with error handling |
| `3b604ff` | fix: app: Skip bare drive letter in EnsureConfigDirectory | Fix `C:` pop_back bug |
| `373c884` | fix: app: Move bare-drive check after pop_back in EnsureConfigDirectory | Refined the same fix |
| `28ad113` | fix: build: Add /utf-8 compiler flag for MSVC encoding | Fix GBK code page 936 issue |

### 2.3 Day 3 - Logger and Heartbeat

| Commit | Message | Purpose |
|---|---|---|
| `7ee0207` | func: app: Add service log writer | Logger module (Info/Warn/Error) |
| `b4b6d89` | fix: app: Fix logger compile errors | Fix `<chrono>` and private member access |
| `00d80d8` | func: app: Add heartbeat worker | HeartbeatWorker with atomic stop flag |
| `35efa45` | docs: app: Add English version of the training project README | README localized to English |

### 2.4 Day 4 - Windows Service Framework

| Commit | Message | Purpose |
|---|---|---|
| `3ca2703` | func: app: Add Windows service framework | ServiceMain / ServiceCtrlHandler / RunServiceBody |
| `12f47d4` | conf: app: Add service control scripts | install / start / stop / query / uninstall .bat |
| `555b50f` | fix: app: Add missing `<windows.h>` and `<atomic>` to service_main.h | Compile fix |
| `b237f8b` | fix: app: Make service_main.h self-contained on Windows | Same fix, refined |
| `35cfcf4` | fix: app: Use SERVICE_TABLE_ENTRYW explicitly for wide-char service name | A/W signature mismatch |
| `8c1984c` | fix: app: Split STOP and SHUTDOWN cases to avoid es.78 fallthrough warning | C++ Core Guidelines compliance |
| `cde4f84` | fix: app: Add const_cast for L"..." literal in SERVICE_TABLE_ENTRYW | Const-correctness fix |
| `484edb0` | fix: conf: Remove Chinese comments from .bat scripts to fix encoding | GBK cmd.exe encoding issue |
| `e770fe8` | fix: conf: Fix binary path in install.bat (one more parent level) | Path depth fix |
| `5e3fea6` | fix: conf: Use absolute path to sc.exe in all scripts | `sc` not in PATH fix |

### 2.5 Day 5 - Cleanup, Documentation, and Reporting

| Commit | Message | Purpose |
|---|---|---|
| `d157512` | style: app: Modify Chinese comments to English | Code style alignment with README |
| `12e73f9` | docs: app: Update the contents of the README | Final README rewrite |
| `d2c6d48` | Merge branch 'week1-cpp-service' of origin | Sync from another contributor |
| (Day 5 final) | `docs: app: Add service test report` | New: `docs/test_report.md` |
| (Day 5 final) | `docs: app: Add week 1 summary` | New: `docs/week1_summary.md` (this file) |

### 2.6 Commit Discipline

- All commit messages follow the `TYPE: app: SUBJECT` template.
- `TYPE` values used: `func`, `fix`, `docs`, `build`, `style`, `conf`, `merge`.
- Subjects are English, short, and describe a single change.
- No "update" / "fix bug" / "test" / "WIP" messages were used.

## 3. Engineering Structure

The repository has a flat layout (sources at the project root, with
`scripts/`, `docs/`, and `third_party/` as subdirectories).

```
DeviceConfigMonitorService/
+- DeviceConfigMonitorService.sln
+- README.md
+- .gitignore
+- .gitattributes
+- x64/                          # Build output (gitignored)
+- DeviceConfigMonitorService/
   +- main.cpp                   # Entry: --console or SCM dispatch
   +- config.h / config.cpp      # AppConfig + JSON load/save
   +- logger.h / logger.cpp      # Logger
   +- heartbeat_worker.h / .cpp  # HeartbeatWorker
   +- service_main.h / .cpp      # ServiceMain + ServiceCtrlHandler
   +- config.example.json        # Sample config
   +- DeviceConfigMonitorService.vcxproj
   +- DeviceConfigMonitorService.vcxproj.filters
   +- scripts/                   # Service control .bat files
   |  +- install.bat
   |  +- uninstall.bat
   |  +- start.bat
   |  +- stop.bat
   |  +- query.bat
   +- docs/                      # Documentation
   |  +- test_report.md
   |  +- week1_summary.md
   +- third_party/               # Vendored third-party libraries
      +- nlohmann/
         +- json.hpp             # nlohmann/json v3.12.0 (header-only)
```

### 3.1 Module Responsibility

| Module | Files | Purpose |
|---|---|---|
| Config | `config.{h,cpp}` | `AppConfig` struct, `LoadConfig`, `SaveConfig`, `PrintConfigSummary` |
| Logger | `logger.{h,cpp}` | Daily-rotated log writer, `Info` / `Warn` / `Error` |
| Heartbeat | `heartbeat_worker.{h,cpp}` | Background `std::thread` writing periodic heartbeat entries |
| Service Framework | `service_main.{h,cpp}` | `ServiceMain`, `ServiceCtrlHandler`, status reporting |
| Main | `main.cpp` | Argument parsing, mode dispatch |
| Scripts | `scripts/*.bat` | Service lifecycle scripts (Administrator required) |
| Docs | `docs/*.md` | Test report and weekly summary |
| Third Party | `third_party/nlohmann/json.hpp` | JSON parsing (single header) |

### 3.2 Build Configuration

- `vcxproj` has 4 configurations: `Debug|x64`, `Debug|x86`, `Release|x64`, `Release|x86`.
- All 4 use `/std:c++17 /utf-8 /EHsc`.
- The `/utf-8` flag is critical to handle non-ASCII characters on
  Chinese Windows (code page 936). See Section 8.1 for details.

## 4. Configuration and JSON

### 4.1 File Location

The configuration file is read from and written to:

```
C:\ProgramData\NanningTraining\DeviceConfigMonitorService\config.json
```

`C:\ProgramData` is used because it is writable by both interactive
users and the LocalSystem account (used by the Windows Service).

### 4.2 Generation, Read, Write Flow

1. **Generation:** On startup, `LoadConfig()` opens
   `config.json` via `std::ifstream`. If the file does not exist, a
   warning is printed, the default `AppConfig` is generated, and
   `SaveConfig()` is called to write the default to disk.
2. **Read:** The file content is read into a `std::string` and
   parsed by `nlohmann::json::parse()`. The result is deserialized
   to an `AppConfig` instance using `j.get<AppConfig>()`.
3. **Write:** `SaveConfig()` first calls `EnsureConfigDirectory()` to
   recursively create the parent directories (skipping bare drive
   letters), then serializes the `AppConfig` to a 2-space-indented
   JSON string and writes it via `std::ofstream` with `std::ios::trunc`.

### 4.3 Exception Handling

`LoadConfig()` catches four exception types:

| Exception | Triggered by | Behaviour |
|---|---|---|
| `json::parse_error` | Malformed JSON | Fall back to defaults |
| `json::out_of_range` | Missing required field | Fall back to defaults |
| `json::type_error` | Field has wrong type | Fall back to defaults |
| `std::exception` | Any other error | Fall back to defaults |

In all four cases, the program logs an `[ERROR]` to stderr with a
human-readable explanation, prints `[INFO] Falling back to default
configuration.`, and continues. The process never crashes on bad
configuration.

## 5. Logging and Heartbeat

### 5.1 Log Path and Format

- **Path:** `AppConfig.LogPath` (default:
  `C:\ProgramData\NanningTraining\DeviceConfigMonitorService\logs`)
- **File naming:** `service-YYYY-MM-DD.log` (one file per day)
- **Line format:** `[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] message`
- **Levels:** `INFO`, `WARN`, `ERROR`
- **Auto-rotation:** Cross-day detection on every write; the file is
  closed and reopened when the day changes.
- **Directory auto-creation:** `EnsureDirectory()` creates missing
  intermediate directories on first write, skipping bare drive
  letters.

### 5.2 Thread Safety

The Logger uses a `std::mutex` to serialize all file and console
writes. Multiple threads (e.g. the heartbeat worker and the main loop)
can call `Logger::Info()` concurrently without corruption.

### 5.3 Startup Logs

When the service starts, the following entries are written:

```
[INFO] Service starting up...
[INFO] Configuration loaded. DeviceId=..., ServiceName=..., LogPath=...
[Logger] Log file: C:\ProgramData\...\logs\service-YYYY-MM-DD.log
[INFO] Logger initialized. LogDir=...
[INFO] Heartbeat worker starting. DeviceId=..., interval=Ns
[INFO] Service entered main loop.
```

### 5.4 Heartbeat Logs

Each heartbeat writes:

```
[INFO] [Heartbeat] DeviceId=..., ServiceName=...
```

- The first heartbeat is emitted immediately, then the worker waits
  for the next interval.
- The wait is split into 200ms slices to keep the stop latency low
  (max 200ms between `Stop` and worker exit).
- If `EnableHeartbeat=false`, only one info line is logged:
  `Heartbeat is disabled (EnableHeartbeat=false).`
- If `HeartbeatIntervalSeconds <= 0`, the actual interval is 5 seconds.

### 5.5 Stop Logs

On graceful shutdown (Ctrl+C in console mode, or
`SERVICE_CONTROL_STOP` / `SHUTDOWN` in service mode):

```
[INFO] Stop signal received. Shutting down...
[INFO] Heartbeat worker stopped.
[INFO] Service stopped.
```

After "Service stopped." the program calls `Logger::Shutdown()` to
flush and close the file. No further writes occur.

## 6. Windows Service

### 6.1 Service Identity

| Field | Value |
|---|---|
| Service name | `DeviceConfigMonitorService` |
| Display name | `Device Config Monitor Service` |
| Start type | `auto` (auto-start with Windows) |
| Service account | `LocalSystem` (default for `sc create`) |

### 6.2 Service Main and Control Handler

`ServiceMain(DWORD, LPWSTR*)` is the entry point called by the SCM:

1. Register `ServiceCtrlHandler` to receive control codes.
2. Initialize `SERVICE_STATUS` fields.
3. Report `SERVICE_START_PENDING` (with `dwControlsAccepted = 0`).
4. Start a `std::thread` running the business body.
5. From the worker thread, report `SERVICE_RUNNING` (with
   `dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN`).
6. Call `RunServiceBody()` which blocks until the stop flag is set.
7. After the business body returns, report `SERVICE_STOPPED`.
8. ServiceMain returns and the process exits.

`ServiceCtrlHandler(DWORD)` handles control codes:

- `SERVICE_CONTROL_STOP`: report `STOP_PENDING`, set the stop flag.
- `SERVICE_CONTROL_SHUTDOWN`: same as STOP.
- Other codes: ignore (default branch).

### 6.3 Install / Start / Query / Stop / Uninstall

All scripts are in `DeviceConfigMonitorService/scripts/` and use
`%~dp0` for relative paths (so the project directory can be moved
freely).

```bat
:: Register
scripts\install.bat

:: Start (auto starts at boot, this is for manual start)
scripts\start.bat

:: Query (sc query) -> STATE, PID, dwControlsAccepted
scripts\query.bat

:: Stop (graceful, sends SERVICE_CONTROL_STOP)
scripts\stop.bat

:: Uninstall (stops first if running, then deletes the service)
scripts\uninstall.bat
```

All scripts require an **Administrator** command prompt, because
`sc.exe` requires `OpenSCManager` privileges. `install.bat` includes
an admin check that writes a temporary file to `%SystemRoot%\System32`
and rolls it back; non-admin runs fail with `[ERROR] This script must
be run as Administrator.`

### 6.4 Why RunServiceBody Is Shared

The business body (`LoadConfig -> Logger::Init -> HeartbeatWorker.Start
-> main loop -> graceful shutdown`) is identical in console and service
mode. The only difference is the source of the stop flag:

| Mode | Stop Flag Source |
|---|---|
| Console | `SIGINT` / `SIGTERM` -> `ConsoleSignalHandler` |
| Service | `SERVICE_CONTROL_STOP` -> `ServiceCtrlHandler` |

Both call the same `RunServiceBody(std::atomic<bool>& stopFlag)`.

## 7. Test Results

Full details and step-by-step outputs are in `docs/test_report.md`.
Summary:

| Category | Total | Passed | Failed |
|---|---|---|---|
| Repository hygiene | 6 | 6 | 0 |
| Build verification | 4 | 4 | 0 |
| Console mode (B-1 to B-5) | 5 | 5 | 0 |
| Script path handling (E-1) | 1 | 1 | 0 |
| Windows Service (Day 4 evidence) | 8 | 8 | 0 |
| **Total** | **24** | **24** | **0** |

### 7.1 Unresolved Issues

None at the time of writing.

## 8. Problem Retrospective

Three major issues were encountered and resolved during the week.
Each is documented here with the original symptom, the diagnosis
process, and the final fix.

### 8.1 MSVC Code Page 936 / UTF-8 Issue (Day 2)

**Symptom:** `cl.exe` reported multiple errors like:

```
warning C4828: The file contains a character that is illegal in the
current source character set (codepage 936).
```

**Diagnosis:** Chinese Windows uses code page 936 (GBK) by default.
MSVC interprets source files in the system code page. Non-ASCII
characters in comments triggered C4828, which under `/W4` becomes
an error.

**Fix:** Added `/utf-8` to the compiler flags in all four build
configurations (`Debug|x64`, `Debug|x86`, `Release|x64`,
`Release|x86`). This tells MSVC to interpret source files as UTF-8
regardless of the system code page.

**Commit:** `28ad113` "fix: build: Add /utf-8 compiler flag for MSVC encoding"

### 8.2 Service Mode Compile Errors (Day 4)

**Symptom:** After adding `service_main.h`, the project produced
100+ compile errors including:

- `error C2027: use of undefined type 'void'`
- `error C2065: 'ServiceMain': undeclared identifier`
- `error C2371: 'DWORD': redefinition`
- `warning C26818 (es.78): fallthrough in switch`
- `error C2440: cannot convert from 'const wchar_t [27]' to 'LPWSTR'`

**Diagnosis:** Five distinct issues were identified and fixed one
at a time:

1. `service_main.h` did not include `<windows.h>`, so types like
   `DWORD`, `LPWSTR`, `SERVICE_STATUS` were incomplete. Fix: add
   the include with an `#ifdef _WIN32` guard.
2. `SERVICE_TABLE_ENTRY` is the A/W dispatch macro; the project's
   service main uses wide-char signatures, so the explicit
   `SERVICE_TABLE_ENTRYW` was required.
3. `ServiceCtrlHandler` had adjacent case labels without `break`,
   triggering C26818 (es.78). Fix: split STOP and SHUTDOWN into
   separate cases, each ending with `break`.
4. `lpServiceName` is `LPWSTR` (non-const) but `L"..."` is a
   `const wchar_t[]`. Fix: `const_cast<LPWSTR>(L"...")`.
5. (Already documented) `.bat` scripts contained Chinese comments
   that GBK cmd.exe misinterpreted.

**Fix:** Five separate `fix: app:` commits, one for each root cause.
The final build is clean under `/W4 /utf-8 /std:c++17`.

**Commits:** `555b50f`, `b237f8b`, `35cfcf4`, `8c1984c`, `cde4f84`

### 8.3 install.bat Encoding and Path Issues (Day 4)

**Symptom 1:** `install.bat` failed with:

```
'ice' 不是内部或外部命令，也不是可运行的程序
[ERROR] sc create failed
```

**Diagnosis:** The script contained Chinese comments in UTF-8
encoding. cmd.exe (code page 936) interpreted the multi-byte
sequences as separate ASCII characters, producing nonsense command
names.

**Fix:** Rewrote all 5 scripts in pure ASCII. All user-facing
documentation moved to the README and the docs/ directory.

**Symptom 2:** `install.bat` reported `Binary not found` even when
the exe was built.

**Diagnosis:** The script was in `scripts/` and the exe was at the
solution root's `x64/Debug/`. The original path used `..\x64\...`
which is only one level up. The correct path is `..\..\x64\...`.

**Fix:** Changed `BIN_PATH` to `%~dp0..\..\x64\Debug\...`.

**Symptom 3:** `install.bat` reported `OpenSCManager failed 5:
拒绝访问` (access denied).

**Diagnosis:** The script was not running as Administrator.

**Fix:** Added an admin check at the top of `install.bat` that
tries to write a temp file to `%SystemRoot%\System32`. If it
fails, the script aborts with a clear `[ERROR] This script must
be run as Administrator.` message. Also hardcoded `SC=C:\Windows\System32\sc.exe`
so the script works even when `sc` is not in the current PATH.

**Commits:** `484edb0`, `e770fe8`, `5e3fea6`

## 9. Self-Evaluation

### 9.1 Most Familiar Areas

- C++ basic syntax, classes, RAII, smart pointers
- `nlohmann/json` library usage (header-only, intuitive API)
- `std::thread` and `std::atomic` for concurrency
- Windows batch scripting (variables, error levels, `sc.exe`)

### 9.2 Areas Needing More Practice

- **Windows API specifics:** `SERVICE_STATUS`, `SERVICE_TABLE_ENTRYW`,
  `RegisterServiceCtrlHandlerW`. The A/W dispatching macros tripped me
  up multiple times. I should review the Windows SDK header
  `winsvc.h` to internalize the wide vs narrow char split.
- **MSBuild / vcxproj internals:** I edited the `.vcxproj` to add
  `/utf-8` and configure include paths. Future projects would
  benefit from a one-time setup script that bakes these in from the
  start, rather than discovering them through compile errors.
- **Async / cancellation patterns:** My stop flag uses a simple
  `std::atomic<bool>` polled every 200ms. For real production
  code I should learn proper cancellation tokens and condition
  variables.

### 9.3 Plan for Week 2

| Topic | Why |
|---|---|
| Event Log integration | The current implementation writes only to a file; production services should also write to the Windows Event Log |
| Service ACL hardening | Default `LocalSystem` is too broad; the right ACL is the minimum required for the service to do its job |
| Config file watching | Reload `config.json` on change without restarting the service |
| Better exception handling in the service body | Right now an uncaught exception in `RunServiceBody` would terminate the process silently; a top-level try/catch should report a meaningful error to the Event Log |
| Unit tests | Add a small GoogleTest or Catch2 suite around `LoadConfig` and the heartbeat interval normalization |

## 10. Attachments

| Filename | Type | Description |
|---|---|---|
| `README.md` | Markdown | Project README, build and run instructions, FAQ |
| `docs/test_report.md` | Markdown | Full test report (Day 5) with step-by-step outputs |
| `docs/week1_summary.md` | Markdown | This document |
| `docs/screenshots/` | Directory | Screenshots taken during final test run (to be added by trainee) |
| `x64/Debug/DeviceConfigMonitorService.exe` | Binary | Compiled binary (1.18 MB) |

### Screenshot Checklist

- [ ] `docs/screenshots/git_status_clean.png`
- [ ] `docs/screenshots/git_log.png` (`git log --oneline --graph`)
- [ ] `docs/screenshots/console_mode.png` (heartbeat output)
- [ ] `docs/screenshots/config_missing.png` (auto-generation)
- [ ] `docs/screenshots/json_malformed.png` (graceful fallback)
- [ ] `docs/screenshots/service_query.png` (`sc query` STATE 4)
- [ ] `docs/screenshots/service_log.png` (startup + heartbeats + stop)
- [ ] `docs/screenshots/readme_rendered.png` (rendered README)

End of report.
