# Bike Dashcam Handoff

This document is the repository's permanent project memory. It reflects the working tree inspected on August 31, 2026. Statements labelled **verified** have execution or test evidence; statements labelled **planned** are not implemented.

## 1. Project Identity

- **Project:** Bike Dashcam
- **POC:** POC-1, software-first technology validation
- **Description:** A C++20 Windows-laptop reference implementation for a future multi-camera motorcycle dashcam. The design uses interfaces so laptop cameras, encoders, storage, and Windows can later be replaced without rewriting the core application.
- **Problem:** Establish whether the desired camera and recording workflow is feasible, then use measured behavior to select embedded hardware rather than selecting a board first.
- **Primary POC objective:** Incrementally validate camera discovery, initialization, capture, recording, storage behavior, recovery, and performance on a laptop.
- **Final POC expectation:** Demonstrate two simultaneous cameras, configurable H.264 MP4 recording and segmentation, circular storage, recovery behavior, monitoring, and stress-test measurements that inform hardware requirements.

## 2. POC Scope

**In scope now:** Windows laptop startup, YAML-subset configuration, console logging, directory preparation, abstract camera management, Windows camera discovery, and a device activation check.

**Out of scope now:** Frame capture, video encoding, MP4 output, GStreamer pipeline integration, HP W100 support as a distinct second-camera milestone, embedded Linux, Raspberry Pi or other board-specific code, cloud services, databases, networking, AI/ML, GPS, display, audio, SOS, and hardware selection.

**Experimental/prototype-only:** The Windows Media Foundation backend briefly activates a selected camera then shuts it down. It validates access; it does not hold a capture session or produce frames. The YAML loader supports only the repository's simple section/scalar subset.

**Production-like:** C++20/CMake structure, RAII cleanup in the Windows backend, dependency injection for camera backends, logging, validation, component status reporting, and deterministic unit tests. **Mocked/simulated:** camera tests use fake backends; recorder, monitoring, events, watchdog recovery, and storage retention are foundations/skeletons rather than complete product features.

## 3. Technology Stack

| Area | Current state |
| --- | --- |
| Language | C++20 |
| Build | CMake 3.25 minimum; project version `0.1.0` |
| Compiler | MSVC x64; verified with MSVC `19.51.36248` |
| OS/runtime | Windows laptop; Windows Media Foundation backend compiled only when `WIN32` |
| Camera API | Windows Media Foundation, COM, Windows SDK libraries `mfplat`, `mf`, and `ole32` |
| Testing | CTest plus a lightweight in-repository C++ harness; GoogleTest is not installed/detected |
| Configuration | Hand-written YAML-subset loader; no external YAML library |
| Logging | In-repository, mutex-protected console logger |
| Multimedia libraries | GStreamer, FFmpeg, OpenCV are probed but not detected or linked; recording is not implemented |
| Storage/database | Local filesystem directories only; no database |
| AI/ML | None |
| Hardware | One Windows video capture device is required for a real Milestone 2 device test; initial target is the integrated webcam |
| External tools | Visual Studio Community 2026 `18.7.2` and bundled CMake `4.3.1-msvc1` were verified |

No environment variables, ports, services, secrets, credentials, network APIs, or database schemas are used.

## 4. Repository Structure

```text
bike_dash_cam/
|- CMakeLists.txt                 # Core build targets and optional dependency probes
|- README.md                      # Project overview and build instructions
|- HANDOFF.md                     # Permanent project memory (this document)
|- config/
|  `- config.yaml                 # Runtime configuration
|- docs/
|  |- HLD.md                      # Product-level high-level design
|  |- ARCHITECTURE.md              # Implemented architecture notes
|  |- DEVELOPMENT.md               # Development environment guidance
|  |- TESTING.md                   # Test strategy and manual camera check
|  `- HARDWARE_REQUIREMENTS.md    # Intentionally unpopulated until measurements exist
|- logs/.gitkeep                  # Placeholder for runtime logs
|- recordings/.gitkeep            # Placeholder for future video output
|- src/
|  |- main.cpp                    # CLI entry point and startup status output
|  |- application/                # DashcamController and StartupReport
|  |- camera/                     # Camera contracts, manager, factory, Windows backend
|  |- configuration/              # Config model and YAML-subset parser
|  |- events/                     # In-process event contracts and manager
|  |- logging/                    # Console logger
|  |- platform/                   # OS/compiler/architecture detection
|  |- recording/                  # Recorder contract, profile validation, segment naming
|  |- storage/                    # Storage contract and directory initialization
|  `- system/                     # Monitor contract/skeleton and watchdog primitive
`- tests/                         # CTest executable and in-repo unit test harness
```

`src/` is core implementation. `config/` is configuration. `tests/` is test-only. `docs/` and this file are supporting documentation. `logs/` and `recordings/` are runtime-output placeholders; they are not currently used for real logging or video.

## 5. Architecture

```text
main.cpp -> DashcamController -> ConfigManager -> DashcamConfig
                  |-> StorageManager -> recordings/ and logs/ directories
                  |-> SystemMonitor, SegmentManager, RecordingManager, Watchdog
                  |-> CameraManager -> ICameraBackend
                  |                     `-> WindowsMediaFoundationCameraBackend -> Windows camera API
                  `-> EventManager -> startup event

DashcamController -> StartupReport -> stdout and process exit code
```

`main.cpp` resolves a configuration path, constructs `DashcamController`, prints its `StartupReport`, and returns `0` only when `report.ready` is true. The controller owns concrete subsystem objects. It accepts injected `ICameraBackend` objects for tests; its default constructor uses `createPlatformCameraBackends()`.

`CameraManager` owns registered backend shared pointers, invokes each backend's `discover()`, logs discovered devices, selects available cameras by case-insensitive friendly-name substring, and calls the matching backend's `initialize()`. It succeeds only when the configured required count initializes. No capture thread or frame path exists.

On Windows the factory creates `WindowsMediaFoundationCameraBackend`. That backend enumerates video capture devices with Media Foundation, uses a device's symbolic link as `CameraDescriptor.id`, and verifies initialization by activating then shutting down an `IMFMediaSource`. Its COM and Media Foundation lifetime is scoped to each call.

## 6. Current Implementation

| Component | Files | Behavior and limitations |
| --- | --- | --- |
| Entry point | `src/main.cpp` | Resolves default or first CLI config path; prints `Configuration`, `Application`, `System`, and overall status. No command-line options beyond config path. |
| Application | `src/application/DashcamController.*`, `StartupReport.hpp` | Loads config; initializes storage/system/segment/recording/camera/watchdog; publishes a startup event. `StartupReport` has version, platform, mode, named checks, and `ready`. |
| Camera contract | `src/camera/ICameraBackend.hpp` | `CameraDescriptor{id,name,backend_name,available}` and virtual `backendName()`, `discover()`, `initialize()`. |
| Camera manager | `src/camera/CameraManager.*` | Registers backends and selects a configured number of matching devices. Discovery/initialization errors cause startup failure. |
| Windows backend | `src/camera/windows/WindowsMediaFoundationCameraBackend.*` | Enumerates/activates Windows camera devices and reports HRESULT errors. It does not request a media type, capture frames, report resolution/FPS capabilities, or retain a source. |
| Configuration | `src/configuration/*` | Loads top-level scalar sections `application`, `cameras`, `recording`, and `logging`. Unknown/invalid keys fail loading. No full YAML support. |
| Logging | `src/logging/*` | Thread-safe timestamped console output at `DEBUG`, `INFO`, `WARN`/`WARNING`, or `ERROR`. `logging.console_enabled` is parsed but not honored by `Logger`. |
| Recording foundation | `src/recording/*` | Defines `IRecorder`, validates `RecordingProfile`, stores profile, and generates segment names. No registered recorder, encoder, pipeline, file writing, or segmentation scheduling. |
| Storage | `src/storage/*` | Resolves/creates configured recordings and logs directories. No capacity, usage, file management, or circular deletion. |
| System | `src/system/*` | `SystemMonitor` only returns a placeholder snapshot; `Watchdog` tracks heartbeat health but performs no recovery. |
| Events | `src/events/*` | Stores/publishes in-process events for future use. No subscriptions, persistence, or external transport. |
| Platform | `src/platform/*` | Detects platform/compiler/architecture for status output. |

## 7. Milestone Status

| Milestone | Status | Implementation | Verification | Remaining Work |
| --------- | ------ | -------------- | ------------ | -------------- |
| 1. Project foundation | COMPLETE | CMake/C++20 project, config/logger, subsystem contracts, startup/status, storage dirs, tests/docs | Build and CTest pass; controller test passes with fake camera backend | Keep stable |
| 2. Camera 1 | IN PROGRESS | Windows Media Foundation discovery and activation backend; configurable selection; camera-manager tests | Automated tests pass. Manual run discovered `ACER HD User Facing` but activation failed `0x80070005` | Enable desktop camera access/release device, then obtain a successful real-device activation run |
| 3. Camera 2 | NOT STARTED | None beyond generic multi-camera manager support | None | Add HP W100 selection/initialization using same contract |
| 4. Dual capture/measurements | NOT STARTED | None | None | Concurrent capture, FPS/drops/CPU/RAM/latency/stability metrics |
| 5. Recording | NOT STARTED | Interfaces/profile validation only | None | GStreamer camera-to-H.264-MP4 pipeline |
| 6. Segmentation | NOT STARTED | Filename/duration validation only | None | Timed configurable segment rotation |
| 7. Circular recording | NOT STARTED | None | None | Capacity limits and oldest non-protected deletion |
| 8. Failure recovery | NOT STARTED | Watchdog/event skeletons only | None | Disconnect/reconnect/application recovery |
| 9. Stress testing | NOT STARTED | None | None | 1/4/8/24-hour runs and data collection |
| 10. Hardware requirements | NOT STARTED | Placeholder document only | None | Derive requirements from measured POC data |

## 8. Completed Work - DO NOT REGRESS

- **Buildable C++20 CMake baseline:** `CMakeLists.txt` defines `bike_dashcam_core`, `bike_dashcam`, and CTest targets. Verified MSVC/NMake build succeeds. Preserve unless a task requires a build-system change.
- **Configuration validation:** `src/configuration/*` loads the current YAML subset and rejects invalid values. `tests/ConfigManagerTests.cpp` verifies valid and invalid files. Preserve the existing error behavior unless explicitly changing the config contract.
- **Startup orchestration and status:** `DashcamController` and `main.cpp` produce component statuses and an exit code. `tests/DashcamControllerTests.cpp` verifies ready startup and directory creation with a fake backend. Preserve this testability.
- **Camera abstraction and selection:** `ICameraBackend`, `CameraManager`, factory, and Windows backend separate platform API from core logic. `tests/CameraManagerTests.cpp` verifies discovery, case-insensitive selection, initialization, and unmatched selection failure. Preserve the interface and injection seam unless explicitly authorized.
- **Windows device discovery:** `WindowsMediaFoundationCameraBackend` successfully enumerated `ACER HD User Facing` during the latest manual execution. Preserve HRESULT diagnostics and RAII cleanup.
- **Automated test harness:** `tests/TestMain.cpp` runs three suites under CTest. The latest verified CTest run passed `1/1` executable test target containing all three suites.

Future agents must preserve every item above unless the current task explicitly requires changing it.

## 9. Current Development State

- **Phase:** POC-1, software-first laptop reference implementation.
- **Current milestone:** Milestone 2, Camera 1.
- **Completed in this milestone:** Abstract backend factory, Windows Media Foundation discovery, stable symbolic-link device identity, configurable name matching, activation attempt, and fake-backend tests.
- **Remaining:** Resolve Windows desktop camera access and complete a successful activation of the integrated webcam. Capture is not part of this milestone.
- **Known current error:** Manual run discovered `ACER HD User Facing`, then `ActivateObject` failed with `HRESULT 0x80070005` (access denied). The process printed `Application: FAIL` and `System Status: NOT READY`.
- **Likely blocker:** Windows camera privacy permission for desktop apps, a camera already in use, or driver/device access. This is environment state, not an automated-test failure.
- **Task file:** `CURRENT_TASK.md` was not present when this handoff was created.

## 10. Interfaces and Contracts

### Camera

```cpp
struct CameraDescriptor { std::string id, name, backend_name; bool available{true}; };
class ICameraBackend {
  virtual std::string backendName() const = 0;
  virtual std::vector<CameraDescriptor> discover() = 0;
  virtual bool initialize(const CameraDescriptor&, std::string& error_message) = 0;
};
```

`CameraDescriptor.id` is a backend-owned stable identifier. For the Windows backend it is a Media Foundation symbolic link. `backend_name` must equal the owning backend's `backendName()`, because `CameraManager` uses that value to route initialization. `initialize()` returns `false` and populates `error_message` on failure.

```cpp
bool CameraManager::initialize(std::size_t required_camera_count,
                               std::string_view preferred_camera_name,
                               std::string& error_message);
```

The manager selects only available descriptors whose `name` contains the preference case-insensitively; an empty preference matches all. It initializes until `required_camera_count` succeeds. Fewer initialized cameras is failure.

### Application, Recording, Storage, System, Events

- `DashcamController::initialize(path)` returns startup readiness; `startupReport()` exposes the result. The injectable constructor takes `std::vector<std::shared_ptr<ICameraBackend>>`.
- `IRecorder::prepare(const RecordingProfile&, std::string&)` is the future recording contract. `RecordingProfile` carries width, height, FPS, codec, segment duration, and continuous flag.
- `IStorageManager::initialize(const DashcamConfig&, std::string&)`, `isReady()`, `recordingsDirectory()`, and `logsDirectory()` define storage behavior.
- `ISystemMonitor` exposes `initialize`, `isReady`, and `snapshot`. `SystemSnapshot.metrics_available` is currently false.
- `IEventManager::publish(const Event&)` and `pendingEventCount()` support local future events. There are no HTTP/API endpoints, ports, or external message formats.

### Configuration file format

`config/config.yaml` is the only runtime configuration file. It supports scalar key/value values under exactly these sections:

| Section | Keys | Meaning |
| --- | --- | --- |
| `application` | `name`, `mode`, `recordings_directory`, `logs_directory` | Identity and runtime directories |
| `cameras` | `expected_camera_count`, `preferred_camera_name`, `target_width`, `target_height`, `target_fps` | Required camera count, optional friendly-name substring, future requested format |
| `recording` | `codec`, `segment_duration_seconds`, `continuous` | Future recording settings; currently validation only |
| `logging` | `level`, `console_enabled` | Logger level; console switch is not implemented |

Positive integers are required where applicable. Booleans accept `true/false`, `yes/no`, `on/off`, or `1/0`. Full YAML collections, nested sections, and root scalars are unsupported.

## 11. Configuration and Environment

Run from the repository root so relative `recordings` and `logs` paths resolve there. The app accepts an optional first positional argument for a config file; otherwise it searches `config/config.yaml` relative to the working directory and executable.

For a real Windows camera test, grant desktop-app camera access in **Windows Settings > Privacy & security > Camera**, and close software that may have reserved the webcam. Do not add a camera index: selection is intentionally name-based rather than index-based. There are no required environment variables or installation steps beyond the verified compiler/build tools and Windows SDK.

## 12. How to Run

The following commands are verified from PowerShell at the repository root. They use the bundled Visual Studio toolchain and NMake generator.

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul && "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --fresh -S . -B build -G "NMake Makefiles"'

cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul && "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build'

.\build\bike_dashcam.exe

& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build --output-on-failure
```

No dependency-install command, lint command, or type-check command is supported by the repository. With `NMake Makefiles`, the verified executable location is `build/bike_dashcam.exe`.

## 13. Verification / Testing

- **Automated suite:** `tests/ConfigManagerTests.cpp`, `tests/CameraManagerTests.cpp`, and `tests/DashcamControllerTests.cpp`, assembled by `tests/TestMain.cpp` and registered as `bike_dashcam_tests` in CTest.
- **Latest automated evidence:** Build completed and `ctest --test-dir build --output-on-failure` passed on August 31, 2026. The single CTest executable contains all three suites.
- **Manual camera test:** Enable desktop camera access, ensure the integrated webcam is free, then run `build/bike_dashcam.exe`. Expected success: an INFO line identifies the discovered camera, followed by `Application: OK` and `System Status: READY`.
- **Latest manual evidence:** Discovery succeeded for `ACER HD User Facing`; activation failed `0x80070005`, so no successful physical activation is yet verified.
- **Testing gaps:** No physical camera success test in CI, no frame/capture test, no format negotiation test, no recording/segmentation/storage-retention/recovery/stress test.

## 14. Expected End-to-End Behavior

When the current POC is working correctly, the executable reads `config/config.yaml`, creates the configured directories, validates recording and segment settings, discovers Windows cameras, chooses one matching `preferred_camera_name` (or the first available when blank), activates it through Media Foundation, publishes a startup event, and prints a READY status. It does not display video, produce a frame, encode video, or create MP4 files.

The intended final POC behavior is broader: two camera streams should be captured simultaneously, measured, encoded to separate segmented MP4 files, retained under storage limits, and recovered after failures. That final behavior is planned, not present.

## 15. Known Issues and Technical Debt

- The real integrated-camera activation currently fails access denied (`0x80070005`).
- `README.md` currently says to run `build\\Debug\\bike_dashcam.exe`, but the verified NMake generator outputs `build\\bike_dashcam.exe`.
- Existing historical portions of `docs/TESTING.md` refer to the Milestone 1 test count/results and August 30 verification; newer Milestone 2 test evidence is in this handoff.
- `logging.console_enabled` is parsed but does not control output.
- `SystemMonitor` reports placeholder metrics only; `Watchdog` has no monitoring loop or recovery action.
- `RecordingManager`, `SegmentManager`, `StorageManager`, and `EventManager` are partial foundations, not full implementations.
- The custom YAML parser intentionally lacks broader YAML compatibility.
- GoogleTest, GStreamer, FFmpeg, and OpenCV remain unavailable/not detected; no dependency has been installed automatically.
- The Windows backend's direct Media Foundation source activation does not prove capture capability, negotiated resolution, or FPS.

## 16. Design Decisions

| Decision | Reason | Consequence |
| --- | --- | --- |
| Software-first laptop reference implementation | Measure real requirements before selecting embedded hardware | No board-specific assumptions in core code |
| C++20/CMake/MSVC | Requested technology and existing Windows environment | Build is portable in structure; Windows camera backend is conditional |
| `ICameraBackend` plus factory | Keep hardware/platform camera code out of core manager/application logic | Future USB/MIPI/Linux backends can be additive |
| Friendly-name preference, not fixed index | Avoid hardcoded camera indexes | Matching can be ambiguous if names are similar; config should be made specific |
| Windows Media Foundation for Milestone 2 | Windows-native camera discovery/activation without new large dependencies | Windows-specific source is required for current real backend |
| Optional multimedia dependency probes | Do not install/use heavy capture/recording dependencies before their milestone | Recording cannot begin until a dependency decision is made |
| Fake backends in tests | Deterministic tests without physical hardware/privacy state | Automated tests do not prove physical device access |
| Lightweight in-repo test harness | GoogleTest was not detected | Test API is intentionally minimal and should not be mistaken for GoogleTest |
| Config-driven dimensions/FPS/segments | Avoid hardcoded product values | Values are currently not negotiated with or applied to a camera |

## 17. Immutable / Protected Areas

## PROTECTED EXISTING IMPLEMENTATION

Future AI agents must preserve completed functionality, avoid unnecessary refactoring, avoid rewriting working components, avoid changing public interfaces without explicit authorization, avoid changing configuration unless required by the current task, and avoid changing architecture unless explicitly required. Prefer additive, incremental implementation. Never delete working functionality because another implementation seems cleaner. Never replace working code with a new framework/library without explicit approval. Never modify completed milestones merely to implement a later milestone unless a dependency makes it unavoidable.

Treat these as protected unless the task explicitly requires a minimal compatible change:

- `CMakeLists.txt` build targets, C++20 setting, warning policy, and optional dependency probes.
- `src/configuration/*` configuration validation contract and `config/config.yaml` existing keys.
- `src/application/DashcamController.*`, `StartupReport.hpp`, and `src/main.cpp` startup/exit status behavior.
- `src/camera/ICameraBackend.hpp`, `CameraManager.*`, `CameraBackendFactory.*`, and `camera/windows/*` abstraction boundary and HRESULT diagnostics.
- All existing files in `tests/`, especially fake-backend injection and current CTest registration.
- Runtime directory initialization in `src/storage/*`.

## 18. Change Management Rules

1. Read `HANDOFF.md` completely before modifying code.
2. Read `CURRENT_TASK.md` completely before modifying code. If it is absent, state that fact and follow the user's active request.
3. Inspect the existing implementation before proposing changes.
4. Identify affected existing files before editing them.
5. Prefer new files/components over modifying stable components when practical.
6. If an existing component must change, make the smallest compatible change.
7. Do not refactor unrelated code.
8. Do not change completed milestones unless explicitly required.
9. Do not introduce dependencies without justification and a scoped need.
10. Preserve backward compatibility wherever practical.
11. Run relevant tests after modifications.
12. Report every file changed.
13. Report existing functionality that may have been affected.
14. If a task conflicts with a design decision, stop and explain the conflict before a major architectural change.

## 19. Future Roadmap

**Planned, in order:**

1. Finish Milestone 2 physical Camera 1 activation verification.
2. Milestone 3: add HP W100 USB webcam through the same interface.
3. Milestone 4: concurrent dual capture and FPS/dropped-frame/CPU/RAM/latency/stability measurement.
4. Milestone 5: choose/integrate GStreamer recording pipeline, H.264 encoding, and MP4 output.
5. Milestone 6: timed configurable segmentation (initial target 300 seconds).
6. Milestone 7: circular storage and oldest non-protected file deletion.
7. Milestone 8: disconnect/reconnect and recording/application recovery.
8. Milestone 9: 1/4/8/24-hour stress testing and metrics collection.
9. Milestone 10: derive CPU/RAM/encoder/camera-interface/storage/power/connectivity requirements, then select embedded hardware.

**Optional/future product work:** display, rider camera, GPS, Bluetooth, Wi-Fi, cellular, microphone/audio, SOS, HD/4K/content-creator configurations.

**Experimental:** alternative FFmpeg/OpenCV use only where they provide a demonstrated benefit; no current implementation commitment.

**Deferred:** SoC/development-board selection and embedded Linux migration until performance data exists.

## 20. AI Continuation Instructions

Read this file first for permanent constraints and actual repository state. Then read `CURRENT_TASK.md` for the active assignment; if it does not exist, use the user's current request and explicitly note the absence. Inspect the relevant protected implementation, make only the smallest additive change needed, run applicable tests, and report changed files plus any impact on completed work.

`HANDOFF.md` is permanent project memory. `CURRENT_TASK.md` is the current implementation assignment.
