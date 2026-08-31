# Bike Dashcam Testing

## Testing strategy

- Unit tests validate core configuration, controller initialization, and deterministic utility behavior.
- Integration tests will be added when camera discovery and recording pipelines exist.
- Camera tests will begin in Milestone 2 when real camera backends are introduced.
- Recording tests will begin when GStreamer-based recording is implemented.
- Failure and recovery tests will begin once disconnect handling and watchdog-driven recovery exist.
- Stress tests will be added progressively for 1-hour, 4-hour, 8-hour, and 24-hour runs after the recording pipeline is stable.

## Current automated tests

- `ConfigManagerTests`: validates the Milestone 1 YAML-subset loader on both valid and invalid input.
- `DashcamControllerTests`: validates startup readiness and creation of configured storage directories.

## Current status

- Planned: camera, recording, failure-recovery, and long-duration stress test suites.
- Implemented: CTest-based automated test executable for Milestone 1.
- Tested: build-time compilation of the test target plus execution of the test binary.
- Verified on August 30, 2026:
- Configure passed with MSVC `19.51.36248` using `vcvars64.bat` and the `NMake Makefiles` generator.
- Build passed for `bike_dashcam_core`, `bike_dashcam`, and `bike_dashcam_tests`.
- Executable run passed and reported `Configuration: OK`, `Application: OK`, `System: OK`, and `System Status: READY`.
- `ctest --test-dir build --output-on-failure` passed with `1/1` tests successful.
- Direct execution of `build/tests/bike_dashcam_tests.exe` passed both `ConfigManagerTests` and `DashcamControllerTests`.
