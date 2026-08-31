# CURRENT TASK

## Task Identity

- **POC:** POC-1, software-first technology validation
- **Milestone:** Milestone 2 - Camera 1 (integrated webcam)
- **Task:** Complete physical Windows camera initialization verification and resolve only a demonstrated implementation defect if one prevents it.
- **Status:** IN PROGRESS
- **Priority:** High - this closes the current milestone before work begins on Camera 2.

## Objective

Verify that the existing Windows Media Foundation backend can discover and initialize the laptop integrated webcam through the existing camera abstraction. The current implementation already discovers the device, but the latest physical run failed at activation with `HRESULT 0x80070005` (access denied). First resolve the local camera-access condition and repeat the test. Change code only if investigation establishes that the repository implementation, rather than Windows privacy/device state, causes the failure.

This verification is required to establish a real, working Camera 1 baseline before adding the HP W100 USB webcam in Milestone 3. See `HANDOFF.md` for the complete project history and constraints.

## Context

The repository is a Windows C++20 reference implementation for a future motorcycle dashcam. Milestone 2 deliberately validates discovery and initialization only; frame capture, video recording, GStreamer, and multi-camera work are later milestones.

The latest application run discovered `ACER HD User Facing` through Windows Media Foundation, then failed to activate it with `0x80070005`. This commonly indicates that Windows desktop-app camera access is disabled, another process owns the camera, or a driver/device restriction applies. This task must not assume the error is a code defect.

## Current State

- `src/camera/ICameraBackend.hpp` defines the protected camera contract: `backendName()`, `discover()`, and `initialize(const CameraDescriptor&, std::string&)`.
- `src/camera/CameraManager.hpp` and `.cpp` discover registered backends, select available devices by case-insensitive `preferred_camera_name`, and require `expected_camera_count` successful initializations.
- `src/camera/windows/WindowsMediaFoundationCameraBackend.cpp` enumerates video capture devices with Media Foundation. It stores the symbolic link in `CameraDescriptor.id`, activates the matching `IMFMediaSource`, then shuts it down. It reports failing HRESULT values.
- `src/camera/CameraBackendFactory.cpp` registers the Windows backend only when `WIN32` is defined.
- `src/application/DashcamController.cpp` initializes all subsystems and sends camera settings from the config into `CameraManager::initialize()`.
- `config/config.yaml` requires one camera and currently leaves `preferred_camera_name` blank, which allows the first available device.
- `tests/CameraManagerTests.cpp` exercises discovery, case-insensitive name selection, success, and failure using a fake backend. `tests/DashcamControllerTests.cpp` validates successful startup with an injected fake backend.
- The latest CTest run passed all three test suites. The physical application run was not READY solely because camera activation was denied.

## Required Outcome

On the Windows laptop, with the integrated webcam available and desktop-app camera access enabled, running `build/bike_dashcam.exe` from the repository root must:

1. Log discovery of at least one Windows camera.
2. Initialize exactly the configured one camera through `ICameraBackend`.
3. Print `Configuration: OK`, `Application: OK`, `System: OK`, and `System Status: READY`.
4. Exit with code `0`.

If Windows access cannot be granted in the current environment, do not falsely claim completion. Record the exact HRESULT, device name, and environment blocker in `docs/TESTING.md` or the final report, and leave the milestone status IN PROGRESS.

## Functional Requirements

- Preserve Windows Media Foundation camera discovery and symbolic-link identification.
- Preserve case-insensitive `preferred_camera_name` matching and blank-preference behavior.
- Preserve clear HRESULT diagnostics when initialization fails.
- Confirm activation succeeds for the configured integrated camera when the OS permits access.
- Keep `expected_camera_count: 1` valid for this milestone.
- Maintain the existing startup report and non-zero exit behavior when camera initialization genuinely fails.
- Keep the fake-backend automated tests independent of physical camera hardware.

## Technical Requirements

- **Language/build:** C++20 and the existing CMake/MSVC build configuration.
- **Platform:** Windows laptop only for the physical device check; the Windows backend remains conditionally compiled under `WIN32`.
- **Camera API:** Existing Windows Media Foundation/COM implementation and linked SDK libraries `mfplat`, `mf`, and `ole32`.
- **Interfaces to preserve:** `ICameraBackend`, `CameraDescriptor`, `CameraManager::initialize`, `DashcamController` injection constructor, and `createPlatformCameraBackends()`.
- **Configuration format:** Keep the existing YAML-subset format and camera keys: `expected_camera_count`, `preferred_camera_name`, `target_width`, `target_height`, and `target_fps`.
- **Dependencies:** Do not add GStreamer, FFmpeg, OpenCV, GoogleTest, or other dependencies for this task. None are needed for device activation validation.
- **Performance:** No capture, FPS, latency, CPU/RAM measurement, or long-run requirement applies in this milestone.
- **Environment:** Before running, close apps using the webcam and enable `Privacy & security > Camera > Let desktop apps access your camera` in Windows Settings.

## Files Expected to Change

Expected for a successful environment-only verification:

- `docs/TESTING.md` - update the documented physical verification result if it succeeds or remains blocked.
- `CURRENT_TASK.md` - update status only if the project process requires task files to reflect completion.

Not expected to change:

- All `src/` implementation files.
- `CMakeLists.txt`.
- `config/config.yaml` unless a specific, user-approved device name needs to be persisted.
- Existing test behavior and interfaces.

Additional files are permitted only if investigation demonstrates a defect in the Windows backend. In that case, restrict changes to the smallest necessary subset of `src/camera/windows/WindowsMediaFoundationCameraBackend.*`, related camera tests, and synchronized documentation. Explain the evidence before making the change.

## Protected Existing Functionality

The following must continue working:

- CMake builds `bike_dashcam_core`, `bike_dashcam`, and `bike_dashcam_tests` with the existing MSVC/NMake flow.
- `ConfigManagerTests`, `CameraManagerTests`, and `DashcamControllerTests` pass.
- `DashcamController` supports dependency-injected fake backends for tests.
- Camera selection remains backend-agnostic and does not introduce camera indices.
- Camera activation failure remains visible in startup output and process status.
- Storage directory initialization and existing configuration validation remain unchanged.

## Implementation Constraints

- Do not rewrite existing working components.
- Do not refactor unrelated code.
- Do not change completed milestone functionality.
- Preserve existing interfaces unless explicitly required by a demonstrated defect.
- Prefer incremental/additive changes.
- Do not introduce unnecessary dependencies.
- Do not modify protected files unless absolutely necessary.
- Do not bypass Windows privacy/security controls or weaken device-access behavior merely to produce a READY status.
- Do not begin frame capture, recording, GStreamer integration, Camera 2 work, or performance measurement.

## Acceptance Criteria

- [ ] The integrated webcam is discovered through `WindowsMediaFoundationCameraBackend`.
- [ ] The configured one camera initializes successfully when Windows grants camera access.
- [ ] `bike_dashcam.exe` prints `Application: OK` and `System Status: READY` and exits `0`.
- [ ] Existing camera selection and HRESULT error reporting remain intact.
- [ ] All existing automated test suites pass.
- [ ] No source changes are made unless a reproducible code defect justifies the smallest possible fix.
- [ ] Any implementation change has targeted test coverage.
- [ ] `docs/TESTING.md` accurately records the final verification state.
- [ ] No unrelated files are modified.

## Verification

From PowerShell at the repository root:

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul && "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build'

& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build --output-on-failure

.\build\bike_dashcam.exe
```

Before the final command, close camera-using applications and enable Windows desktop-app camera access. Expected automated result: `100% tests passed`. Expected physical result: an INFO line for the discovered camera, all three component checks `OK`, `System Status: READY`, and shell exit code `0`.

If a code change is necessary, run a fresh configure/build as well:

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul && "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --fresh -S . -B build -G "NMake Makefiles" && "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build'
```

## Out of Scope

- Adding or initializing the HP W100 USB webcam as Camera 2.
- Any simultaneous or persistent capture.
- H.264 encoding, GStreamer, FFmpeg, OpenCV, MP4 generation, or recording.
- Segment rotation, circular storage, file deletion, recovery behavior, or stress tests.
- Camera format negotiation, resolution/FPS capability reporting, or performance measurement.
- Linux, ARM, Raspberry Pi, embedded-board, cloud, networking, GPS, audio, display, SOS, AI/ML, or hardware-selection work.
- Broad documentation cleanup unrelated to this exact verification task.

## Final Report Required

When finished, report:

1. What was implemented or verified.
2. Files created.
3. Files modified.
4. Files deleted (normally none).
5. Tests added.
6. Tests executed.
7. Test results, including the physical application exit code.
8. Any known issues, including exact HRESULT/device details if blocked.
9. Any deviations from this task.
10. Whether Milestone 1 and existing Milestone 2 functionality remain functional.
