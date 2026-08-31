#include "application/DashcamController.hpp"

#include <filesystem>
#include <iostream>
#include <vector>

namespace {

std::filesystem::path resolveConfigPath(const int argc, char* argv[]) {
    if (argc > 1) {
        return argv[1];
    }

    const std::filesystem::path default_config{"config/config.yaml"};
    if (std::filesystem::exists(default_config)) {
        return default_config;
    }

    if (argc > 0 && argv[0] != nullptr) {
        const std::filesystem::path executable_directory = std::filesystem::absolute(argv[0]).parent_path();
        const std::vector<std::filesystem::path> candidates = {
            executable_directory / "config/config.yaml",
            executable_directory / "../config/config.yaml",
            executable_directory / "../../config/config.yaml"
        };

        for (const auto& candidate : candidates) {
            if (std::filesystem::exists(candidate)) {
                return candidate;
            }
        }
    }

    return default_config;
}

void printStatusLine(const bike_dashcam::application::ComponentStatus& status) {
    std::cout << status.name << ": " << (status.ok ? "OK" : "FAIL");
    if (!status.ok && !status.detail.empty()) {
        std::cout << " (" << status.detail << ')';
    }
    std::cout << '\n';
}

}  // namespace

int main(int argc, char* argv[]) {
    bike_dashcam::application::DashcamController controller{};
    const bool ready = controller.initialize(resolveConfigPath(argc, argv));
    const auto& report = controller.startupReport();

    std::cout << "BIKE DASHCAM POC-1\n";
    std::cout << "Version: " << report.version << '\n';
    std::cout << "Platform: " << report.platform << '\n';
    std::cout << "Mode: " << report.mode << "\n\n";

    for (const auto& check : report.checks) {
        printStatusLine(check);
    }

    std::cout << "\nSystem Status: " << (report.ready ? "READY" : "NOT READY") << '\n';
    return ready ? 0 : 1;
}
