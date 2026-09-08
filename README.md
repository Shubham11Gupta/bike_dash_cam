# Bike Dashcam

## POC-1 — Software-First Technology Validation

Motorcycle dashcam software platform designed around a hardware-independent
architecture.

POC-1 validates the core recording architecture on a Windows development
laptop before moving toward Raspberry Pi and eventually production embedded
hardware.

## POC-1 Goals

- Multi-camera recording
- Front/rear video streams
- Continuous recording
- Automatic video segmentation
- Circular storage management
- Camera monitoring and recovery
- System monitoring
- Configuration-driven operation
- Logging
- Application/watchdog recovery
- Long-duration reliability testing

## Architecture

```text
Application Layer
    ↓
Service Layer
    ↓
Hardware Abstraction Layer
    ↓
Platform / OS
    ↓
Hardware
```

Major services:
- Dashcam Controller
- Camera Manager
- Recording Manager
- Storage Manager
- Event Manager
- System Monitor
- Configuration Manager
- Watchdog / Recovery

## Development Environment

- Windows 10/11
- C++17
- MSVC x64
- CMake
- vcpkg
- GStreamer
- yaml-cpp

See `docs/INSTALLATION.md` for setup instructions.

## Project Structure

```text
bike_dash_cam/
├── CMakeLists.txt
├── README.md
├── requirements.txt
├── vcpkg.json
├── config/
│   └── config.yaml
├── docs/
│   └── INSTALLATION.md
├── src/
│   ├── configuration/
│   │   ├── ConfigManager.hpp
│   │   └── ConfigManager.cpp
│   └── recording/
│       ├── Recorder.hpp
│       └── Recorder.cpp
├── tests/
├── recordings/
└── logs/
```

## Build

```cmd
cmake -S . -B build -G "Visual Studio 18 2026" -DCMAKE_TOOLCHAIN_FILE="C:/Program Files/Microsoft Visual Studio/18/Community/VC/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release
```

Run:
```cmd
build\Release\bike-dashcam.exe
```

## Development Status

Validated:
- CMake/MSVC project
- GStreamer C++ integration
- GStreamer H.264 recording
- MP4 output
- Video segmentation experiments
- vcpkg manifest integration
- yaml-cpp integration
- ConfigManager
- Typed configuration structures

Next:
1. Configuration validation and defaults
2. Configuration-driven Recorder
3. Camera abstraction
4. Simulated camera backend
5. Multi-camera recording
6. Segment manager
7. Storage manager and circular recording
8. System monitoring
9. Watchdog/recovery
10. Event manager
11. Dashcam controller/state machine
12. Failure injection and stress testing

## Design Principle

The POC is intentionally built incrementally. The architecture remains
hardware-independent so the implementation can later move from Windows to
Linux/embedded Linux with minimal changes to application and service layers.
