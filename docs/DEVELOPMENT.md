# Bike Dashcam Development Guide

## Verified local environment

The following environment was verified on August 30, 2026:

- Operating system: Windows
- IDE/toolchain: Visual Studio Community 2026 `18.7.2`
- Compiler: MSVC `19.51.36248` for `x64`
- CMake: `4.3.1-msvc1`

## Dependency status

- Required for Milestone 1: C++20 compiler and CMake.
- Preferred later: GStreamer, FFmpeg, OpenCV, and GoogleTest.
- Current local detection on August 30, 2026: GStreamer not found, FFmpeg not found, OpenCV not found, GoogleTest not found in the active shell or the common Windows locations checked during setup.

## Repository structure

- `config/`: runtime configuration.
- `docs/`: project and engineering documentation.
- `logs/`: runtime logs placeholder directory.
- `recordings/`: recording output placeholder directory.
- `src/`: application source code.
- `tests/`: automated tests.

## Configure and build

Run the commands below from the repository root:

```powershell
cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul && "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --fresh -S . -B build -G "NMake Makefiles"'

cmd /c 'call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul && "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build'
```

## Verified build note

- The `cmake` executable was not available on the default `PATH` in the active shell.
- The Visual Studio 2026 generator existed, but on August 30, 2026 it did not auto-detect a usable C++ compiler in this shell.
- The verified Milestone 1 build flow therefore uses `vcvars64.bat` plus `NMake Makefiles`, still with MSVC as the compiler.

## Run

```powershell
.\build\Debug\bike_dashcam.exe
```

If the executable is started from a different working directory, pass the config path explicitly:

```powershell
.\build\Debug\bike_dashcam.exe .\config\config.yaml
```

## Test

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' `
  --test-dir build --output-on-failure
```

## Development notes

- Keep the core application platform-independent where possible.
- Add hardware-specific integrations only behind stable interfaces.
- Do not require GStreamer, FFmpeg, OpenCV, or board-specific code for Milestone 1 builds.
- Treat documentation updates as part of each architectural change.
