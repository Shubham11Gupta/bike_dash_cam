# Bike Dashcam POC-1 — Installation Guide

## 1. Overview

POC-1 is a software-first technology validation running on a Windows
development laptop. The architecture is intended to remain hardware-
independent so it can later move to Linux and embedded hardware.

Current stack:
- C++17
- MSVC x64
- CMake
- vcpkg
- yaml-cpp
- GStreamer
- Git

## 2. Clone the Repository

```cmd
git clone <REPOSITORY_URL>
cd bike_dash_cam
```

Replace `<REPOSITORY_URL>` with the GitHub repository URL.

## 3. Install Visual Studio

Install Visual Studio Community or a compatible edition.

Select:
**Desktop development with C++**

Ensure these components are installed:
- C++ core desktop features
- MSVC C++ build tools for x64/x86
- Windows 10/11 SDK
- C++ CMake tools for Windows
- vcpkg

Verify from an x64 Native Tools Command Prompt:
```cmd
cl
```

## 4. Install CMake

Verify:
```cmd
cmake --version
```

POC-1 was developed with CMake 4.4.3.

## 5. Install GStreamer

Install the official Windows **MSVC x86_64** packages:
- Runtime
- Development headers/libraries

POC-1 uses:
```text
C:\Program Files\gstreamer\1.0\msvc_x86_64
```

Add this directory to PATH:
```text
C:\Program Files\gstreamer\1.0\msvc_x86_64\bin
```

Open a new terminal and verify:
```cmd
gst-launch-1.0 --version
where gst-launch-1.0
```

The installer may create:
```text
GSTREAMER_1_0_ROOT_MSVC_X86_64
```
with the value:
```text
C:\Program Files\gstreamer\1.0\msvc_x86_64
```

## 6. Verify GStreamer

```cmd
gst-launch-1.0 videotestsrc num-buffers=100 ! autovideosink
```

Verify H.264 support:
```cmd
gst-inspect-1.0 x264enc
```

The application uses the GStreamer C API. `gst-launch-1.0` is used for
pipeline testing/debugging, not as an application subprocess.

## 7. Verify Git

```cmd
git --version
```

## 8. vcpkg

This project uses **vcpkg manifest mode**.

The repository contains:
```text
vcpkg.json
```

The manifest declares:
```text
yaml-cpp
```

Do not install yaml-cpp using classic global vcpkg mode.

## 9. Configure

From the project root, use an MSVC-capable terminal:

```cmd
cmake -S . -B build -G "Visual Studio 18 2026" -DCMAKE_TOOLCHAIN_FILE="C:/Program Files/Microsoft Visual Studio/18/Community/VC/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

If vcpkg is installed elsewhere, replace the toolchain path with:
```text
<vcpkg-root>/scripts/buildsystems/vcpkg.cmake
```

CMake/vcpkg should automatically install dependencies from `vcpkg.json`.

## 10. Build

```cmd
cmake --build build --config Release
```

Executable:
```text
build\Release\bike-dashcam.exe
```

## 11. Run

```cmd
build\Release\bike-dashcam.exe
```

The current POC initializes GStreamer, loads the YAML configuration,
starts the recorder, processes the test video, and exits cleanly.

## 12. Configuration

Configuration file:
```text
config\config.yaml
```

Current structure:
```yaml
system:
  log_level: INFO

cameras:
  front:
    enabled: true
    resolution: 1920x1080
    fps: 30
  rear:
    enabled: true
    resolution: 1920x1080
    fps: 30

recording:
  codec: h264
  segment_duration: 300

storage:
  max_usage_percent: 90
  recording_path: ./recordings
```

## 13. Dummy Camera Inputs

Video files can be used as simulated camera sources during POC development.

Example development assets:
```text
sample-30s.mp4
sample-20s.mp4
```

These are test assets and should not be assumed to exist in a fresh clone
unless intentionally committed or distributed separately.

## 14. Environment Verification

```cmd
cl
cmake --version
git --version
gst-launch-1.0 --version
gst-inspect-1.0 x264enc
cmake --build build --config Release
build\Release\bike-dashcam.exe
```

## 15. Clean Reconfiguration

If the generator/toolchain/vcpkg configuration changes:

```cmd
rmdir /s /q build
```

Then rerun the configure command.

Do not manually edit files inside `build/`.

## 16. Git Recommendations

Normally commit:
```text
CMakeLists.txt
README.md
requirements.txt
vcpkg.json
config/
docs/
src/
tests/
```

Normally ignore:
```text
build/
*.exe
*.obj
*.pdb
*.lib
*.dll
recordings/
logs/
```

## 17. Development Workflow

Normal:
```cmd
git pull
cmake --build build --config Release
build\Release\bike-dashcam.exe
```

After dependency/CMake changes:
```cmd
cmake -S . -B build -G "Visual Studio 18 2026" -DCMAKE_TOOLCHAIN_FILE="C:/Program Files/Microsoft Visual Studio/18/Community/VC/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release
```

## 18. Future Platform Migration

Current:
```text
Windows POC
    ↓
Linux development
    ↓
Embedded Linux
    ↓
Production controller hardware
```

The application/service architecture should remain independent of camera
and hardware implementations. GStreamer and HAL/backend abstractions are
key parts of maintaining that portability.
