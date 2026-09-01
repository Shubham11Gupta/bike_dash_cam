cmake_minimum_required(VERSION 3.25)

project(bike_dashcam VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

option(BIKE_DASHCAM_BUILD_TESTS "Build Bike Dashcam test targets" ON)

function(bike_dashcam_apply_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive- /EHsc)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

list(APPEND BIKE_DASHCAM_GSTREAMER_HINTS
    "C:/Program Files/GStreamer/1.0/msvc_x86_64/bin"
    "C:/gstreamer/1.0/msvc_x86_64/bin"
)
list(APPEND BIKE_DASHCAM_FFMPEG_HINTS
    "C:/ffmpeg/bin"
    "C:/Program Files/ffmpeg/bin"
    "C:/tools/ffmpeg/bin"
)
list(APPEND BIKE_DASHCAM_OPENCV_HINTS
    "C:/opencv/build/x64/vc17/bin"
    "C:/opencv/build/x64/vc16/bin"
)

find_program(BIKE_DASHCAM_GSTREAMER_EXECUTABLE
    NAMES gst-launch-1.0
    HINTS ${BIKE_DASHCAM_GSTREAMER_HINTS}
)
find_program(BIKE_DASHCAM_FFMPEG_EXECUTABLE
    NAMES ffmpeg
    HINTS ${BIKE_DASHCAM_FFMPEG_HINTS}
)
find_program(BIKE_DASHCAM_OPENCV_VERSION_EXECUTABLE
    NAMES opencv_version
    HINTS ${BIKE_DASHCAM_OPENCV_HINTS}
)
find_path(BIKE_DASHCAM_GTEST_INCLUDE_DIR
    NAMES gtest/gtest.h
    HINTS
        "C:/vcpkg/installed/x64-windows"
        "C:/src/vcpkg/installed/x64-windows"
    PATH_SUFFIXES include
)

if(BIKE_DASHCAM_GSTREAMER_EXECUTABLE)
    set(BIKE_DASHCAM_GSTREAMER_STATUS "found")
else()
    set(BIKE_DASHCAM_GSTREAMER_STATUS "not found")
endif()

if(BIKE_DASHCAM_FFMPEG_EXECUTABLE)
    set(BIKE_DASHCAM_FFMPEG_STATUS "found")
else()
    set(BIKE_DASHCAM_FFMPEG_STATUS "not found")
endif()

if(BIKE_DASHCAM_OPENCV_VERSION_EXECUTABLE)
    set(BIKE_DASHCAM_OPENCV_STATUS "found")
else()
    set(BIKE_DASHCAM_OPENCV_STATUS "not found")
endif()

if(BIKE_DASHCAM_GTEST_INCLUDE_DIR)
    set(BIKE_DASHCAM_GTEST_STATUS "found")
else()
    set(BIKE_DASHCAM_GTEST_STATUS "not found")
endif()

message(STATUS "Optional dependency probe")
message(STATUS "  GStreamer: ${BIKE_DASHCAM_GSTREAMER_STATUS}")
message(STATUS "  FFmpeg: ${BIKE_DASHCAM_FFMPEG_STATUS}")
message(STATUS "  OpenCV: ${BIKE_DASHCAM_OPENCV_STATUS}")
message(STATUS "  GoogleTest headers: ${BIKE_DASHCAM_GTEST_STATUS}")

add_library(bike_dashcam_core
    src/application/DashcamController.cpp
    src/camera/CameraBackendFactory.cpp
    src/camera/DummyCameraBackend.cpp
    src/camera/CameraManager.cpp
    src/configuration/ConfigManager.cpp
    src/events/EventManager.cpp
    src/logging/Logger.cpp
    src/platform/PlatformInfo.cpp
    src/recording/RecordingManager.cpp
    src/recording/SegmentManager.cpp
    src/storage/StorageManager.cpp
    src/system/SystemMonitor.cpp
    src/system/Watchdog.cpp
)

if(WIN32)
    target_sources(bike_dashcam_core PRIVATE
        src/camera/windows/WindowsMediaFoundationCameraBackend.cpp
    )
    target_link_libraries(bike_dashcam_core PRIVATE mfplat mf mfreadwrite ole32)
endif()

target_include_directories(bike_dashcam_core PUBLIC "${PROJECT_SOURCE_DIR}/src")
target_compile_definitions(bike_dashcam_core PUBLIC BIKE_DASHCAM_VERSION="${PROJECT_VERSION}")
bike_dashcam_apply_warnings(bike_dashcam_core)

add_executable(bike_dashcam src/main.cpp)
target_link_libraries(bike_dashcam PRIVATE bike_dashcam_core)
bike_dashcam_apply_warnings(bike_dashcam)

if(BIKE_DASHCAM_BUILD_TESTS AND BUILD_TESTING)
    add_subdirectory(tests)
endif()
