#include "TestCases.hpp"

#include <iostream>
#include <utility>
#include <vector>

namespace {

using TestFunction = bool (*)();

struct NamedTest {
    const char* name;
    TestFunction function;
};

}  // namespace

int main() {
    const std::vector<NamedTest> tests = {
        {"ConfigManagerTests", &bike_dashcam::tests::runConfigManagerTests},
        {"CameraManagerTests", &bike_dashcam::tests::runCameraManagerTests},
        {"DashcamControllerTests", &bike_dashcam::tests::runDashcamControllerTests},
    };

    int failed_count = 0;

    for (const auto& test : tests) {
        std::cout << "[ RUN      ] " << test.name << '\n';
        const bool passed = test.function();
        if (passed) {
            std::cout << "[       OK ] " << test.name << '\n';
        } else {
            std::cout << "[  FAILED  ] " << test.name << '\n';
            ++failed_count;
        }
    }

    if (failed_count == 0) {
        std::cout << "[  PASSED  ] " << tests.size() << " test suite(s)\n";
        return 0;
    }

    std::cout << "[  FAILED  ] " << failed_count << " test suite(s)\n";
    return 1;
}
