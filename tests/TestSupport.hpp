#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

namespace bike_dashcam::tests {

class TestContext {
public:
    explicit TestContext(std::string name) : name_(std::move(name)) {
    }

    void expect(bool condition, std::string_view message) {
        if (condition) {
            return;
        }

        ++failure_count_;
        std::cerr << "  failure in " << name_ << ": " << message << '\n';
    }

    int failureCount() const {
        return failure_count_;
    }

private:
    std::string name_;
    int failure_count_{0};
};

inline std::filesystem::path makeUniqueTestDirectory(std::string_view prefix) {
    const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() /
        (std::string{"bike_dashcam_"} + std::string{prefix} + "_" + std::to_string(timestamp));

    std::filesystem::create_directories(directory);
    return directory;
}

inline void writeTextFile(const std::filesystem::path& file_path, const std::string& content) {
    std::ofstream output(file_path, std::ios::binary);
    output << content;
}

inline std::string quotedPath(const std::filesystem::path& path) {
    return "\"" + path.generic_string() + "\"";
}

}  // namespace bike_dashcam::tests
