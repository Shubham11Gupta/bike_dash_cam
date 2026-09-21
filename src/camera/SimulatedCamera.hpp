#pragma once

#include "Camera.hpp"

class SimulatedCamera : public Camera
{
public:
    explicit SimulatedCamera(const std::string& file_path);

    bool start() override;
    bool stop() override;
    bool isHealthy() const override;

    std::string getPipelineSource() const override;
    // POC-only fault injection.
    void simulateFailure();

private:
    std::string file_path_;
    bool running_;
};