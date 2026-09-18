cmake_minimum_required(VERSION 3.20)

project(bike_dashcam
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------------------------------
# GStreamer
# --------------------------------------------------

set(GSTREAMER_ROOT
    "C:/Program Files/gstreamer/1.0/msvc_x86_64"
)

set(GSTREAMER_INCLUDE_DIR
    "${GSTREAMER_ROOT}/include/gstreamer-1.0"
)

set(GLIB_INCLUDE_DIR
    "${GSTREAMER_ROOT}/include/glib-2.0"
)

set(GLIB_CONFIG_INCLUDE_DIR
    "${GSTREAMER_ROOT}/lib/glib-2.0/include"
)

set(GSTREAMER_LIB_DIR
    "${GSTREAMER_ROOT}/lib"
)

# Tell the compiler where GStreamer headers are located
include_directories(
    "${GSTREAMER_INCLUDE_DIR}"
    "${GLIB_INCLUDE_DIR}"
    "${GLIB_CONFIG_INCLUDE_DIR}"
)

# Tell the linker where GStreamer libraries are located
link_directories(
    "${GSTREAMER_LIB_DIR}"
)

# --------------------------------------------------
# Application
# --------------------------------------------------

find_package(yaml-cpp CONFIG REQUIRED)

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
)

# Link GStreamer
target_link_libraries(bike-dashcam
    PRIVATE
        gstreamer-1.0.lib
        glib-2.0.lib
        yaml-cpp::yaml-cpp
    )