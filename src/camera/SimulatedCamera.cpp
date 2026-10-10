#include "SimulatedCamera.hpp"

#include "../logging/Logger.hpp"

SimulatedCamera::SimulatedCamera(const std::string& file_path)
    : file_path_(file_path),
      running_(false)
{
}

bool SimulatedCamera::start()
{
    if (file_path_.empty())
    {
        Logger::error(
            "Simulated camera file path is empty."
        );

        return false;
    }

    running_ = true;

    Logger::info(
        "Simulated camera started."
    );

    return true;
}

bool SimulatedCamera::stop()
{
    running_ = false;

    Logger::info(
        "Simulated camera stopped."
    );

    return true;
}

bool SimulatedCamera::isHealthy() const
{
    return running_;
}

std::string SimulatedCamera::getPipelineSource() const
{
    return "filesrc location=\"" + file_path_ + "\"";
}

void SimulatedCamera::simulateFailure()
{
    Logger::info(
        "[TEST] Simulating camera failure."
    );

    running_ = false;
}