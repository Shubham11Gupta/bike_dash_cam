# Bike Dashcam Testing

## Testing strategy

- Unit tests validate core configuration, controller initialization, and deterministic utility behavior.
- Integration tests will expand when capture and recording pipelines exist.
- Camera tests begin in Milestone 2 with a Windows Media Foundation device discovery and activation check.
- Recording tests will begin when GStreamer-based recording is implemented.
- Failure and recovery tests will begin once disconnect handling and watchdog-driven recovery exist.
- Stress tests will be added progressively for 1-hour, 4-hour, 8-hour, and 24-hour runs after the recording pipeline is stable.

## Current automated tests

- `ConfigManagerTests`: validates the YAML-subset loader on both valid and invalid input.
- `CameraManagerTests`: validates backend discovery, case-insensitive friendly-name selection, initialization, and failure when no selected device exists.
- `DashcamControllerTests`: validates startup readiness and creation of configured storage directories.

## Current status

- Planned: capture, recording, failure-recovery, and long-duration stress test suites.
- Implemented: CTest-based automated test executable, fake-backend camera manager tests, and a manual Windows webcam activation check through the application.

## Milestone 2 Manual Camera Test

1. Close applications that may be using the webcam.
2. In Windows Settings, enable camera access for desktop apps under `Privacy & security > Camera`.
3. Optionally set `cameras.preferred_camera_name` in `config/config.yaml` to a unique portion of the integrated webcam's name.
4. Run `build/bike_dashcam.exe` from the repository root.

Expected result: the application discovers and activates one Windows camera, then prints `Application: OK` and `System Status: READY`. A failure includes the Windows HRESULT from device activation; it normally indicates camera permission, an occupied device, or driver/device failure.
- Tested: build-time compilation of the test target plus execution of the test binary.
- Verified on August 30, 2026:
- Configure passed with MSVC `19.51.36248` using `vcvars64.bat` and the `NMake Makefiles` generator.
- Build passed for `bike_dashcam_core`, `bike_dashcam`, and `bike_dashcam_tests`.
- Executable run passed and reported `Configuration: OK`, `Application: OK`, `System: OK`, and `System Status: READY`.
- `ctest --test-dir build --output-on-failure` passed with `1/1` tests successful.
- Direct execution of `build/tests/bike_dashcam_tests.exe` passed both `ConfigManagerTests` and `DashcamControllerTests`.


## Commands
Use Powershell for this
- C:\GitHub\bike_dash_cam
- cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul && "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --fresh -S . -B build -G "NMake Makefiles" && "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build'
- .\build\bike_dashcam.exe
- & 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build --output-on-failure
