#include "platform/PlatformInfo.hpp"

namespace bike_dashcam::platform {

PlatformInfo detectPlatformInfo() {
    PlatformInfo info{};

#if defined(_WIN32)
    info.operating_system = "Windows";
#elif defined(__linux__)
    info.operating_system = "Linux";
#elif defined(__APPLE__)
    info.operating_system = "macOS";
#else
    info.operating_system = "Unknown";
#endif

#if defined(_MSC_VER)
    info.compiler = "MSVC";
#elif defined(__clang__)
    info.compiler = "Clang";
#elif defined(__GNUC__)
    info.compiler = "GCC";
#else
    info.compiler = "Unknown";
#endif

#if defined(_M_X64) || defined(__x86_64__)
    info.architecture = "x64";
#elif defined(_M_ARM64) || defined(__aarch64__)
    info.architecture = "arm64";
#elif defined(_M_IX86) || defined(__i386__)
    info.architecture = "x86";
#else
    info.architecture = "Unknown";
#endif

    return info;
}

}  // namespace bike_dashcam::platform
