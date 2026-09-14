#include "SimulatedCamera.hpp"

#include <iostream>

SimulatedCamera::SimulatedCamera(const std::string& file_path)
    : file_path_(file_path),
      running_(false)
{
}

bool SimulatedCamera::start()
{
    if (file_path_.empty())
    {
        std::cerr << "Simulated camera file path is empty."
                  << std::endl;

        return false;
    }

    running_ = true;

    std::cout << "Simulated camera started."
              << std::endl;

    return true;
}

bool SimulatedCamera::stop()
{
    running_ = false;

    std::cout << "Simulated camera stopped."
              << std::endl;

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