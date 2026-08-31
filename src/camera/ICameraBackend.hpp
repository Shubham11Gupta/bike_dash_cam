#pragma once

#include <string>
#include <vector>

namespace bike_dashcam::camera {

struct CameraDescriptor {
    std::string id;
    std::string name;
    std::string backend_name;
    bool available{true};
};

class ICameraBackend {
public:
    virtual ~ICameraBackend() = default;

    virtual std::string backendName() const = 0;
    virtual std::vector<CameraDescriptor> discover() = 0;
    virtual bool initialize(const CameraDescriptor& camera, std::string& error_message) = 0;
};

}  // namespace bike_dashcam::camera
