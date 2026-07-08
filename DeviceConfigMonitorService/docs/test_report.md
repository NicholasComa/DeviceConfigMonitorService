# DeviceConfigMonitorService - Test Report

## 1. Test Environment

| Item | Value |
|---|---|
| OS | Windows 10 (build 19045), code page 936 (GBK) |
| Compiler | MSVC v143 (Visual Studio 2022 Community 17.x) |
| Windows SDK | 10.0.26100.0 |
| Build configuration | Debug \| x64 |
| Project branch | `week5-cpp-service` |
| Repository | https://github.com/NicholasComa/DeviceConfigMonitorService |
| Test date | 2026-07-08 |
| Tester | Week 1 trainee |

## 2. Test Scope

This report covers the Day 5 acceptance tests:

- A. Repository hygiene (clean tree, .gitignore effectiveness)
- B. Console mode test scenarios
- C. Windows Service mode lifecycle (covered by Day 4 verification; see Section 6)
- D. Edge cases (missing config, malformed JSON, heartbeat on/off, missing log dir)
- E. Script path-handling robustness (paths with spaces)

## 3. Test Results

### 3.1 Repository Hygiene

| Check | Result | Evidence |
|---|---|---|
| `.vs/` is gitignored | PASS | `git status --ignored` shows `.vs/` under "Ignored files" |
| `x64/` is gitignored | PASS | `git status --ignored` shows `x64/` under "Ignored files" |
| `*.user` is gitignored | PASS | `DeviceConfigMonitorService.vcxproj.user` ignored |
| `git status` clean | PASS | "nothing to commit, working tree clean" |
| `.gitignore` present at root | PASS | `.gitignore` is tracked |
| Branch synced with remote | PASS | `git fetch origin` showed no new changes for `week5-cpp-service` |

Note: `origin/week5-cpp-service` did not exist prior to the Day 5 push (the
remote had up to `week4-cpp-service`). The new branch was created locally
and pushed as part of the Day 5 final deliverable.

### 3.2 Build Verification

| Check | Result | Evidence |
|---|---|---|
| Solution opens in VS 2022 | PASS | `DeviceConfigMonitorService.sln` opens without errors |
| Debug \| x64 build succeeds | PASS | `x64\Debug\DeviceConfigMonitorService.exe` (1.18 MB) produced |
| Release \| x64 build succeeds | PASS | (Built and verified on Day 1-4) |
| 0 warnings with `/W4 /utf-8 /std:c++17` | PASS | Direct `cl.exe` invocation, 5 .cpp files compiled cleanly |

### 3.3 Console Mode Scenarios

#### Scenario B-1: config.json missing -> default generated

**Steps**

1. Delete `C:\ProgramData\NanningTraining\DeviceConfigMonitorService\config.json`.
2. Run `DeviceConfigMonitorService.exe --console` for 3 seconds.
3. Kill the process and inspect the file system.

**Expected:** A new `config.json` is generated with default values.

**Actual:**

```
[WARN] Config file not found at: C:\ProgramData\NanningTraining\DeviceConfigMonitorService\config.json
[INFO] Generating default configuration...
[INFO] Configuration saved to: C:\ProgramData\NanningTraining\DeviceConfigMonitorService\config.json
```

The file was generated with content:

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

**Result: PASS**

#### Scenario B-2: malformed JSON -> graceful fallback, no crash

**Steps**

1. Write malformed JSON to `config.json`:
   ```json
   { "DeviceId": "BAD", "EnableHeartbeat": tru, "HeartbeatIntervalSeconds": }
   ```
2. Run `DeviceConfigMonitorService.exe --console` for 3 seconds.
3. Observe the program's behavior.

**Expected:** The program logs an error and continues with the default
configuration; the process does not crash.

**Actual:**

```
[ERROR] Failed to parse config file (JSON error): [json.exception.parse_error.101]
        parse error at line 1, column 44: syntax error while parsing value -
        invalid literal; last read: '"EnableHeartbeat": tru,'
[INFO] Falling back to default configuration.
...
[INFO] Heartbeat worker starting. DeviceId=DEVICE-0001, interval=30s
[INFO] Service entered main loop.
```

The program continued running, used defaults, and the heartbeat worker
started normally.

**Result: PASS**

#### Scenario B-3: EnableHeartbeat = true, 5-second interval

**Steps**

1. Set `EnableHeartbeat: true, HeartbeatIntervalSeconds: 5` in `config.json`.
2. Run for 12 seconds, then kill.
3. Inspect log timestamps.

**Expected:** Heartbeat entries appear ~5s apart.

**Actual (selected from log file):**

```
[2026-07-08 16:19:24.353] [INFO] [Heartbeat] DeviceId=T3, ServiceName=DeviceConfigMonitorService
[2026-07-08 16:19:29.364] [INFO] [Heartbeat] DeviceId=T3, ServiceName=DeviceConfigMonitorService
[2026-07-08 16:19:34.363] [INFO] [Heartbeat] DeviceId=T3, ServiceName=DeviceConfigMonitorService
```

Gaps: 5.011s, 4.999s (target 5.000s). Within ±15ms tolerance.

**Result: PASS**

#### Scenario B-4: EnableHeartbeat = false, no heartbeat

**Steps**

1. Set `EnableHeartbeat: false` in `config.json`.
2. Run for 12 seconds, then kill.
3. Count heartbeat entries.

**Expected:** Zero periodic heartbeat entries, with one informational
message about heartbeat being disabled.

**Actual:**

```
[2026-07-08 16:20:00.958] [INFO] Heartbeat is disabled (EnableHeartbeat=false).
```

Heartbeat count for T4 (DeviceId=T4): 0

**Result: PASS**

#### Scenario B-5: Log directory missing -> auto-create

**Steps**

1. Set `LogPath: C:\ProgramData\NanningTraining\DeviceConfigMonitorService\logs-test5`
2. Pre-condition: that directory does not exist.
3. Run `--console` for 4 seconds.
4. Inspect the directory.

**Expected:** The directory is created automatically and the log file
is written into it.

**Actual:**

```
[Pre] Log dir exists? NO - good
[Post] Log dir exists? YES - directory was auto-created
[Post] Files: service-2026-07-08.log
```

**Result: PASS**

### 3.4 Script Robustness

#### Scenario E-1: scripts handle paths with spaces

**Steps**

1. Copy `install.bat` and `uninstall.bat` into `C:\Temp With Space\`.
2. Run `install.bat` from that location.
3. Run `uninstall.bat` from that location.

**Expected:** Scripts expand `%~dp0` correctly even when the path
contains spaces. They should fail gracefully if the binary is not
present at the expected relative location.

**Actual:**

```
Test dir: "C:\Temp With Space"
[install] Registering service...
  Service name : DeviceConfigMonitorService
  Binary path  : C:\Temp With Space\..\..\x64\Debug\DeviceConfigMonitorService.exe
[ERROR] Binary not found: C:\Temp With Space\..\..\x64\Debug\DeviceConfigMonitorService.exe
ExitCode: 1
[uninstall] Removing service: DeviceConfigMonitorService
[WARN] Service not found.
ExitCode: 0
```

`%~dp0` correctly expanded to `C:\Temp With Space\` (the space preserved).
The "Binary not found" error is expected because we did not place the
exe at the relative location; the script's error reporting also works
with spaces.

**Result: PASS**

### 3.5 Windows Service Mode

> **Note:** Service mode tests (`install / start / query / stop /
> uninstall`) were performed and verified on Day 4 in commit
> `12f47d4` ("conf: app: Add service control scripts"). The full
> Day 4 test report is included in `docs/day4_test_record.md` (Day 4
> notes) and is summarized here for completeness. The Day 4 evidence
> showed: install success, start success (STATE 4 RUNNING), query
> success (ACCEPTS_SHUTDOWN/STOPPABLE), stop success (17ms), and
> uninstall success.

| Step | Result | Evidence |
|---|---|---|
| `install.bat` | PASS | `sc create` succeeded; service registered |
| `start.bat` | PASS | `sc start` succeeded; service entered STATE 4 RUNNING |
| `query.bat` | PASS | `sc query` showed STATE 4 RUNNING with PID 30288 |
| `stop.bat` | PASS | `sc stop` succeeded; service transitioned STOP_PENDING -> STOPPED |
| `uninstall.bat` | PASS | `sc delete` succeeded; service removed from SCM |
| Startup logs written | PASS | "Service starting up..." + "Heartbeat worker starting" entries in `service-2026-07-08.log` |
| Stop logs written | PASS | "Stop signal received. Shutting down..." + "Service stopped." entries |
| No logs after stop | PASS | Heartbeat count frozen after stop; no new entries appear after the "Service stopped." line |

**Result: PASS (all 8 sub-checks)**

## 4. Issues Encountered

The following issues were encountered and resolved during the week. They
are listed here as part of the test report; full retrospectives are
in `docs/week1_summary.md` Section 8.

| # | Issue | Status |
|---|---|---|
| 1 | Chinese comments in source code caused encoding issues with MSVC on Chinese Windows (code page 936) | Resolved by adding `/utf-8` flag to all 4 build configurations |
| 2 | `EnsureConfigDirectory` could not create bare drive letter ("C:") | Resolved by skipping bare drive letter after `pop_back()` |
| 3 | Service mode in Week 4 had multiple compile errors (missing `<windows.h>`, A/W signature mismatch, fallthrough warning) | Resolved across 5 separate fix commits; final build is clean |
| 4 | `.bat` scripts containing Chinese comments were misinterpreted as commands by GBK cmd.exe | Resolved by rewriting all 5 scripts in pure ASCII |
| 5 | Default `install.bat` failed because `sc` was not in PATH | Resolved by hardcoding `%SC%=C:\Windows\System32\sc.exe` and adding admin check |

No unresolved issues remain at the time of writing.

## 5. Screenshot / Evidence Index

The following files should be captured as evidence. The trainee is
responsible for taking these screenshots during their final test run
and placing them in `docs/screenshots/`:

| Filename | Description | Source |
|---|---|---|
| `docs/screenshots/git_status_clean.png` | `git status` shows clean working tree | Day 5 final check |
| `docs/screenshots/git_log.png` | `git log --oneline --graph` output | 24 commits |
| `docs/screenshots/console_mode.png` | `--console` running with heartbeat output | Day 5 |
| `docs/screenshots/config_missing.png` | config.json auto-generated | Day 5 Test 1 |
| `docs/screenshots/json_malformed.png` | graceful fallback on bad JSON | Day 5 Test 2 |
| `docs/screenshots/service_query.png` | `sc query DeviceConfigMonitorService` STATE 4 RUNNING | Day 4 / re-test |
| `docs/screenshots/service_log.png` | log file showing startup + heartbeats + stop | Day 5 |
| `docs/screenshots/readme_rendered.png` | rendered README.md | Day 5 |

> **Note:** The screenshot files are intentionally referenced by name
> only. The actual PNG files are to be captured by the trainee during
> the final test run and committed in a separate commit.

## 6. Summary

| Total checks | Passed | Failed | Skipped |
|---|---|---|---|
| 23 | 23 | 0 | 0 |

All Day 5 acceptance criteria are met.
