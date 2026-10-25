#include "SystemMonitor.hpp"

#include "../logging/Logger.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

// ------------------------------------------------------------------
// CPU usage
// ------------------------------------------------------------------

double SystemMonitor::getCpuUsage()
{
    if (simulate_cpu_failure_)
    {
        return 100.0;
    }

    static unsigned long long previous_idle = 0;
    static unsigned long long previous_total = 0;
    static bool first_sample = true;

    std::ifstream stat_file("/proc/stat");

    if (!stat_file.is_open())
    {
        Logger::error("Failed to retrieve CPU usage.");
        return -1.0;
    }

    std::string cpu_label;
    unsigned long long user = 0;
    unsigned long long nice = 0;
    unsigned long long system = 0;
    unsigned long long idle = 0;
    unsigned long long iowait = 0;
    unsigned long long irq = 0;
    unsigned long long softirq = 0;
    unsigned long long steal = 0;

    stat_file >> cpu_label
              >> user
              >> nice
              >> system
              >> idle
              >> iowait
              >> irq
              >> softirq
              >> steal;

    if (!stat_file || cpu_label != "cpu")
    {
        Logger::error("Failed to parse CPU statistics.");
        return -1.0;
    }

    const unsigned long long idle_total = idle + iowait;

    const unsigned long long total =
        user + nice + system + idle + iowait +
        irq + softirq + steal;

    if (first_sample)
    {
        previous_idle = idle_total;
        previous_total = total;
        first_sample = false;
        return 0.0;
    }

    if (total < previous_total || idle_total < previous_idle)
    {
        previous_idle = idle_total;
        previous_total = total;
        return 0.0;
    }

    const unsigned long long total_delta = total - previous_total;
    const unsigned long long idle_delta = idle_total - previous_idle;

    previous_idle = idle_total;
    previous_total = total;

    if (total_delta == 0)
    {
        return 0.0;
    }

    const double idle_percent =
        static_cast<double>(idle_delta) /
        static_cast<double>(total_delta) * 100.0;

    return std::clamp(100.0 - idle_percent, 0.0, 100.0);
}

// ------------------------------------------------------------------
// Memory usage
// ------------------------------------------------------------------

double SystemMonitor::getMemoryUsage()
{
    if (simulate_memory_failure_)
    {
        return 100.0;
    }

    std::ifstream meminfo_file("/proc/meminfo");

    if (!meminfo_file.is_open())
    {
        Logger::error("Failed to retrieve memory usage.");
        return -1.0;
    }

    unsigned long long total_memory_kb = 0;
    unsigned long long available_memory_kb = 0;

    std::string line;

    while (std::getline(meminfo_file, line))
    {
        std::istringstream line_stream(line);
        std::string key;
        unsigned long long value = 0;

        if (!(line_stream >> key >> value))
        {
            continue;
        }

        if (key == "MemTotal:")
        {
            total_memory_kb = value;
        }
        else if (key == "MemAvailable:")
        {
            available_memory_kb = value;
        }
    }

    if (total_memory_kb == 0 || available_memory_kb > total_memory_kb)
    {
        Logger::error("Failed to parse memory statistics.");
        return -1.0;
    }

    const double used_memory_kb =
        static_cast<double>(total_memory_kb - available_memory_kb);

    const double memory_usage =
        used_memory_kb / static_cast<double>(total_memory_kb) * 100.0;

    return std::clamp(memory_usage, 0.0, 100.0);
}

// ------------------------------------------------------------------
// Test failure simulation
// ------------------------------------------------------------------

void SystemMonitor::simulateCpuFailure()
{
    Logger::info("[SYSTEM TEST] Simulating CPU failure.");
    simulate_cpu_failure_ = true;
    Logger::info("[SYSTEM TEST] CPU failure simulated.");
}

void SystemMonitor::simulateMemoryFailure()
{
    Logger::info("[SYSTEM TEST] Simulating memory failure.");
    simulate_memory_failure_ = true;
    Logger::info("[SYSTEM TEST] Memory failure simulated.");
}

void SystemMonitor::clearSimulatedFailure()
{
    simulate_cpu_failure_ = false;
    simulate_memory_failure_ = false;

    Logger::info("[SYSTEM TEST] Simulated system failure cleared.");
}