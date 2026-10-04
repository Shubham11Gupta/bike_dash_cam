#include <csignal>
#include <iostream>

#include <gst/gst.h>

#include "src/controller/DashcamController.hpp"
#include "src/logging/Logger.hpp"

namespace
{
    DashcamController* controller = nullptr;

    void handleSignal(int signal)
    {
        if (signal == SIGINT && controller != nullptr)
        {
            controller->requestShutdown();
        }
    }
}

int main()
{
    gst_init(nullptr, nullptr);

    if (!Logger::initialize("INFO", "./logs/dashcam.log"))
    {
        std::cerr
            << "Failed to initialize logger."
            << std::endl;

        gst_deinit();
        return 1;
    }

    Logger::info("Dashcam application starting.");

    DashcamController dashcam_controller;

    controller = &dashcam_controller;

    std::signal(SIGINT, handleSignal);

    if (!dashcam_controller.initialize())
    {
        Logger::error("Dashcam initialization failed.");

        gst_deinit();
        return 1;
    }

    int result =
        dashcam_controller.run();

    controller = nullptr;

    gst_deinit();

    Logger::info("Application finished.");

    return result;
}