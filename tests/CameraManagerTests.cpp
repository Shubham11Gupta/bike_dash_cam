#include "TestCases.hpp"
#include "TestSupport.hpp"

#include "camera/CameraManager.hpp"
#include "logging/Logger.hpp"

#include <memory>

namespace bike_dashcam::tests {
namespace {

class FakeCameraBackend final : public camera::ICameraBackend {
public:
    std::string backendName() const override {
        return "Fake camera backend";
    }

    std::vector<camera::CameraDescriptor> discover() override {
        return cameras;
    }

    bool initialize(const camera::CameraDescriptor& camera, std::string& error_message) override {
        if (camera.name == failing_camera_name) {
            error_message = "simulated initialization failure";
            return false;
        }
        initialized_ids.push_back(camera.id);
        return true;
    }

    bool captureFor(const camera::CameraDescriptor& camera, std::chrono::milliseconds,
                    camera::CaptureStatistics& statistics, std::string&) override {
        statistics.camera_name = camera.name;
        statistics.frame_count = 30;
        statistics.fps = 30.0;
        statistics.healthy = true;
        return true;
    }

    std::vector<camera::CameraDescriptor> cameras;
    std::vector<std::string> initialized_ids;
    std::string failing_camera_name;
};

}  // namespace

bool runCameraManagerTests() {
    TestContext context{"CameraManagerTests"};
    logging::Logger logger;
    camera::CameraManager manager{logger};

    auto backend = std::make_shared<FakeCameraBackend>();
    backend->cameras = {
        {"camera-1", "Integrated Camera", backend->backendName(), true},
        {"camera-2", "USB Camera", backend->backendName(), true},
    };
    manager.registerBackend(backend);

    std::string error_message;
    context.expect(manager.initialize(1, "integrated", "", error_message), "matching camera should initialize");
    context.expect(manager.discoveredCameraCount() == 2, "both cameras should be discovered");
    context.expect(manager.initializedCameraCount() == 1, "one camera should be initialized");
    context.expect(backend->initialized_ids.size() == 1 && backend->initialized_ids.front() == "camera-1", "name selection should be case-insensitive");

    error_message.clear();
    context.expect(!manager.initialize(1, "missing", "", error_message), "unmatched camera preference should fail");
    context.expect(!error_message.empty(), "failed selection should explain the error");

    return context.failureCount() == 0;
}

}  // namespace bike_dashcam::tests
