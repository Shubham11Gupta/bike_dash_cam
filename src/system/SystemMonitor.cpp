#include "SystemMonitor.hpp"

#include <windows.h>

#include <iostream>

double SystemMonitor::getCpuUsage()
{
    if (simulate_cpu_failure_)
    {
        return 100.0;
    }
    static ULONGLONG previous_idle = 0;
    static ULONGLONG previous_kernel = 0;
    static ULONGLONG previous_user = 0;
    static bool first_sample = true;

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

    const ULONGLONG current_idle =
        (static_cast<ULONGLONG>(idle_time.dwHighDateTime) << 32) |
        idle_time.dwLowDateTime;

    const ULONGLONG current_kernel =
        (static_cast<ULONGLONG>(kernel_time.dwHighDateTime) << 32) |
        kernel_time.dwLowDateTime;

    const ULONGLONG current_user =
        (static_cast<ULONGLONG>(user_time.dwHighDateTime) << 32) |
        user_time.dwLowDateTime;

    // First sample establishes the baseline.
    if (first_sample)
    {
        previous_idle = current_idle;
        previous_kernel = current_kernel;
        previous_user = current_user;

        first_sample = false;

        return 0.0;
    }

    // Protect against invalid counter movement.
    if (current_idle < previous_idle ||
        current_kernel < previous_kernel ||
        current_user < previous_user)
    {
        previous_idle = current_idle;
        previous_kernel = current_kernel;
        previous_user = current_user;

        return 0.0;
    }

    const ULONGLONG idle_delta =
        current_idle - previous_idle;

    const ULONGLONG kernel_delta =
        current_kernel - previous_kernel;

    const ULONGLONG user_delta =
        current_user - previous_user;

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

    double cpu_usage = 100.0 - idle_percent;

    // Defensive bounds.
    if (cpu_usage < 0.0)
    {
        cpu_usage = 0.0;
    }

    if (cpu_usage > 100.0)
    {
        cpu_usage = 100.0;
    }

    return cpu_usage;
}

double SystemMonitor::getMemoryUsage()
{
    if (simulate_memory_failure_)
    {
        return 100.0;
    }

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
// ------------------------------------------------------------
// Test failure simulation
// ------------------------------------------------------------

void SystemMonitor::simulateCpuFailure()
{
    std::cout
        << "[SYSTEM TEST] Simulating CPU failure."
        << std::endl;

    simulate_cpu_failure_ = true;

    std::cout
        << "[SYSTEM TEST] CPU failure simulated."
        << std::endl;
}

void SystemMonitor::simulateMemoryFailure()
{
    std::cout
        << "[SYSTEM TEST] Simulating memory failure."
        << std::endl;

    simulate_memory_failure_ = true;

    std::cout
        << "[SYSTEM TEST] Memory failure simulated."
        << std::endl;
}

void SystemMonitor::clearSimulatedFailure()
{
    simulate_cpu_failure_ = false;
    simulate_memory_failure_ = false;

    std::cout
        << "[SYSTEM TEST] Simulated system failure cleared."
        << std::endl;
}