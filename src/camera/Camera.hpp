#pragma once

#include <string>

class Camera
{
public:
    virtual ~Camera() = default;

    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool isHealthy() const = 0;

    virtual std::string getPipelineSource() const = 0;
};