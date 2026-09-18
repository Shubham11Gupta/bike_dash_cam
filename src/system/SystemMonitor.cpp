#include "SystemMonitor.hpp"

#include <windows.h>

#include <iostream>

double SystemMonitor::getCpuUsage()
{
    static ULARGE_INTEGER previous_idle;
    static ULARGE_INTEGER previous_kernel;
    static ULARGE_INTEGER previous_user;

    FILETIME idle_time;
    FILETIME kernel_time;
    FILETIME user_time;

    if (!GetSystemTimes(
            &idle_time,
            &kernel_time,
            &user_time))
    {
        std::cerr
            << "Failed to retrieve CPU usage."
            << std::endl;

        return -1.0;
    }

    ULARGE_INTEGER current_idle;
    ULARGE_INTEGER current_kernel;
    ULARGE_INTEGER current_user;

    current_idle.LowPart = idle_time.dwLowDateTime;
    current_idle.HighPart = idle_time.dwHighDateTime;

    current_kernel.LowPart = kernel_time.dwLowDateTime;
    current_kernel.HighPart = kernel_time.dwHighDateTime;

    current_user.LowPart = user_time.dwLowDateTime;
    current_user.HighPart = user_time.dwHighDateTime;

    static bool first_sample = true;

    if (first_sample)
    {
        previous_idle = current_idle;
        previous_kernel = current_kernel;
        previous_user = current_user;

        first_sample = false;

        return 0.0;
    }

    const ULONGLONG idle_delta =
        current_idle.QuadPart -
        previous_idle.QuadPart;

    const ULONGLONG kernel_delta =
        current_kernel.QuadPart -
        previous_kernel.QuadPart;

    const ULONGLONG user_delta =
        current_user.QuadPart -
        previous_user.QuadPart;

    const ULONGLONG total_delta =
        kernel_delta + user_delta;

    previous_idle = current_idle;
    previous_kernel = current_kernel;
    previous_user = current_user;

    if (total_delta == 0)
    {
        return 0.0;
    }

    const double idle_percent =
        static_cast<double>(idle_delta) /
        static_cast<double>(total_delta) *
        100.0;

    return 100.0 - idle_percent;
}

double SystemMonitor::getMemoryUsage()
{
    MEMORYSTATUSEX memory_status;

    memory_status.dwLength =
        sizeof(MEMORYSTATUSEX);

    if (!GlobalMemoryStatusEx(&memory_status))
    {
        std::cerr
            << "Failed to retrieve memory usage."
            << std::endl;

        return -1.0;
    }

    return static_cast<double>(
        memory_status.dwMemoryLoad
    );
}