# Bike Dashcam Architecture

## Implemented scope

This document reflects the software architecture implemented through Milestone 2. Camera capture, recording pipelines, encoding, recovery, and stress tooling are not implemented yet and should be treated as planned only.

## Source layout

- `src/application`: startup orchestration and process-level status reporting.
- `src/configuration`: runtime configuration model and a YAML-subset loader.
- `src/logging`: thread-safe console logging foundation with log levels.
- `src/platform`: platform and compiler detection helpers.
- `src/camera`: camera abstraction, camera manager, backend factory, and Windows Media Foundation backend.
- `src/recording`: recorder interface, recording profile validation, and segment naming foundation.
- `src/storage`: storage abstraction and directory initialization logic.
- `src/system`: system monitor skeleton and watchdog timing primitive.
- `src/events`: generic in-process event model and event manager skeleton.
- `tests`: executable test harness, configuration tests, and camera-manager unit tests.

## Startup flow

- `main.cpp` resolves the configuration path.
- `DashcamController` loads the configuration and builds a startup report.
- `StorageManager` creates configured `recordings` and `logs` directories.
- `SystemMonitor` marks monitoring readiness for Milestone 1, but does not yet collect resource metrics.
- `SegmentManager` validates the configured segment duration.
- `RecordingManager` validates the recording profile without binding to a concrete encoder or pipeline.
- `CameraManager` discovers available backends, selects cameras by configurable friendly-name preference, and asks the owning backend to initialize them. It does not capture frames.
- `EventManager` records a startup event for future event-driven expansion.

## Dependency strategy

- C++20 and CMake are the required build foundation.
- GStreamer, FFmpeg, and OpenCV remain optional through Milestone 2 because recording and image processing are not implemented yet.
- GoogleTest is preferred for future testing, but it was not detected in the local environment on August 30, 2026. The project therefore uses CTest with a lightweight in-repo test harness so the baseline remains buildable without downloading new dependencies.

## Configuration model

- Runtime configuration is stored in `config/config.yaml`.
- The current loader supports the subset needed through Milestone 2: top-level sections with scalar key/value pairs.
- Supported sections today are `application`, `cameras`, `recording`, and `logging`.
- Unknown keys or invalid values are treated as configuration errors.
- Full YAML feature coverage is not implemented yet and should not be assumed.

## Current design decisions

- No Raspberry Pi or board-specific code exists in the core application.
- No camera indexes, resolutions, segment lengths, or storage paths are hardcoded in the executable logic; they come from configuration defaults.
- Managers validate readiness independently so later milestones can swap in platform-specific backends behind stable interfaces.
- The logging foundation is intentionally small and thread-safe so it can later grow into structured file logging without rewriting subsystem call sites.

## Planned extension points

- Add a second Windows camera selection for the HP W100 behind the existing `ICameraBackend`.
- Add a GStreamer-based recording backend behind `IRecorder`.
- Extend `SystemMonitor` with CPU, RAM, FPS, dropped-frame, and storage-usage metrics.
- Add circular retention logic to `StorageManager`.
- Add recovery loops and health supervision on top of `Watchdog` and `EventManager`.
