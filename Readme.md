# Bike Dashcam

Bike Dashcam is a software-first Proof of Concept for a multi-camera motorcycle dashcam platform. The Windows laptop build in this repository is the reference implementation for POC-1 and is intentionally designed around hardware abstraction so we can measure real software requirements before selecting embedded hardware.

## Current scope

- Milestones 1 and 2 implemented: project foundation plus Windows camera discovery and initialization through an abstract camera backend.
- Milestones 3 and beyond remain planned: a second camera, dual capture, recording, segmentation, circular storage, recovery, and stress testing.

## Verified environment

The current foundation was prepared against this laptop environment on August 30, 2026:

- Visual Studio Community 2026 `18.7.2`
- MSVC `19.51.36248` for `x64`
- CMake `4.3.1-msvc1` from the Visual Studio installation

At that time, GStreamer, FFmpeg, OpenCV, and GoogleTest were not detected on the active `PATH` or in the common Windows locations probed by the project.

## Build

From the repository root:

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul && "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --fresh -S . -B build -G "NMake Makefiles"'

cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul && "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build'
```

This `NMake Makefiles` flow is the verified path on August 30, 2026. The Visual Studio 2026 generator was present, but in this shell it did not auto-detect a usable C++ compiler.

## Run

```powershell
.\build\Debug\bike_dashcam.exe
```

The executable looks for `config/config.yaml` automatically and accepts an explicit config path as its first argument.

## Test

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' `
  --test-dir build --output-on-failure
```

## Documentation

Project documentation lives under `docs/`:

- `HLD.md`
- `ARCHITECTURE.md`
- `DEVELOPMENT.md`
- `TESTING.md`
- `HARDWARE_REQUIREMENTS.md`
