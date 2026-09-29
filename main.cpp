#include <csignal>
#include <iostream>

#include <gst/gst.h>

#include "src/controller/DashcamController.hpp"

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

    DashcamController dashcam_controller;

    controller = &dashcam_controller;

    std::signal(SIGINT, handleSignal);

    if (!dashcam_controller.initialize())
    {
        std::cerr
            << "Dashcam initialization failed."
            << std::endl;

        gst_deinit();
        return 1;
    }

    int result =
        dashcam_controller.run();

    controller = nullptr;

    gst_deinit();

    std::cout
        << "Application finished."
        << std::endl;

    return result;
}