#pragma once

class SystemMonitor
{
public:
    SystemMonitor() = default;
    ~SystemMonitor() = default;

    double getCpuUsage();
    double getMemoryUsage();
};