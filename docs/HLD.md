# Bike Dashcam High-Level Design

## Purpose

Bike Dashcam is a software-first Proof of Concept for a multi-camera motorcycle dashcam product family targeted at the Indian market. The laptop implementation in this repository is the reference implementation for POC-1 and is used to derive future embedded hardware requirements from measured software behavior.

## Product family roadmap

- Basic: front and rear recording.
- Basic+: front and rear recording plus a display.
- Basic Pro: front, rear, rider camera, and display.
- Advanced: GPS, Bluetooth, Wi-Fi, and cellular connectivity.
- Advanced Pro: HD cameras, microphone or audio, and SOS features.
- Content Creator: higher-end 4K and advanced recording capabilities.

## POC phases

- POC-1 Phase 1: technology validation on a Windows laptop with the integrated webcam and one USB webcam.
- POC-1 later phases: dual-camera capture, recording, segmentation, circular storage, failure recovery, and stress testing.
- Post-POC-1: derive CPU, RAM, encoder, storage, and interface requirements before choosing an embedded SoC or board.

## Architectural principles

- Hardware abstraction first. The core application must not depend directly on a specific camera type, encoder, storage medium, or operating system.
- Software-first validation. We measure real requirements on a laptop before making processor decisions.
- Platform portability. Core logic should stay largely portable across Windows, Linux, and future ARM embedded Linux targets.
- Modular growth. Milestones should remain buildable and testable without forcing later-stage dependencies too early.

## Logical component model

- DashcamController coordinates startup, shutdown, and subsystem orchestration.
- ConfigManager loads runtime settings from YAML configuration.
- CameraManager owns camera backend abstractions and will later coordinate device discovery and capture.
- RecordingManager and Recorder abstractions will later own encode and file output flows.
- SegmentManager will manage fixed-duration recording segments.
- StorageManager will manage storage paths, capacity, and circular retention policies.
- SystemMonitor will expose system-health and performance metrics.
- EventManager provides a generic event channel for future SOS, camera fault, GPS, and recovery signals.
- Watchdog and recovery components will detect and recover from failure conditions in later milestones.
- Platform abstractions isolate OS and toolchain differences from the core application flow.

## Data flow target

- Configuration is loaded at startup from YAML.
- DashcamController initializes storage, monitoring, camera backends, recording, and future recovery hooks.
- Camera backends will feed capture frames into a recording pipeline.
- Recording output will be segmented and stored with retention policies enforced by StorageManager.
- System and fault events will be published through EventManager for operator visibility and automated recovery logic.

## Migration strategy

- Current capture devices: Windows laptop integrated webcam and HP W100 USB webcam.
- Future camera migration: laptop webcam to USB to MIPI or CSI camera backends behind the same camera interface.
- Future encoder migration: software encoding to hardware encoder backends behind recording abstractions.
- Future OS migration: Windows development build to Linux and ARM embedded Linux with platform-specific adapters.
- Future storage migration: laptop filesystem to SSD, eMMC, or SD storage through StorageManager abstractions.

## Status as of August 30, 2026

- Planned: camera discovery, camera initialization, dual-camera capture, H.264 recording, segmentation, circular recording, recovery, and long-duration stress testing.
- Implemented: Milestones 1 and 2, including the foundation plus Windows integrated-camera discovery and initialization through the camera abstraction.
- Tested: configuration parsing, startup readiness flow, and storage directory initialization.
- Verified: Windows configure, build, executable startup, and automated tests completed on August 30, 2026. See `docs/TESTING.md` for the current verification record.
