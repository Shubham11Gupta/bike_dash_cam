cmake_minimum_required(VERSION 3.20)

project(bike_dashcam
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------
# Platform validation
# ------------------------------------------------------------

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR
        "This CMake configuration is intended for Linux."
    )
endif()

# ------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------

find_package(PkgConfig REQUIRED)

pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET
    gstreamer-1.0
    gobject-2.0
    glib-2.0
)

find_package(yaml-cpp CONFIG REQUIRED)

# ------------------------------------------------------------
# Application
# ------------------------------------------------------------

add_executable(bike-dashcam
    main.cpp
    src/recording/Recorder.cpp
    src/configuration/ConfigManager.cpp
    src/platform/SoftwareEncoderBackend.cpp
    src/camera/SimulatedCamera.cpp
    src/camera/CameraManager.cpp
    src/recording/RecordingManager.cpp
    src/recording/SegmentManager.cpp
    src/storage/StorageManager.cpp
    src/system/SystemMonitor.cpp
    src/events/EventManager.cpp
    src/watchdog/Watchdog.cpp
    src/controller/DashcamController.cpp
    src/logging/Logger.cpp
    src/platform/NvidiaEncoderBackend.cpp
)

target_include_directories(bike-dashcam PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(bike-dashcam PRIVATE
    PkgConfig::GSTREAMER
    yaml-cpp::yaml-cpp
)

# ------------------------------------------------------------
# Compiler warnings
# ------------------------------------------------------------

target_compile_options(bike-dashcam PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)