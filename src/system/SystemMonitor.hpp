#pragma once

class SystemMonitor
{
public:
    SystemMonitor() = default;
    ~SystemMonitor() = default;

    double getCpuUsage();
    double getMemoryUsage();

    // ------------------------------------------------------------
    // Test failure simulation
    // ------------------------------------------------------------

    void simulateCpuFailure();
    void simulateMemoryFailure();
    void clearSimulatedFailure();

private:
    bool simulate_cpu_failure_ = false;
    bool simulate_memory_failure_ = false;
};