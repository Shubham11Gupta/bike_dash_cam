#include "NvidiaEncoderBackend.hpp"

#include "../logging/Logger.hpp"

std::string NvidiaEncoderBackend::getEncoderElement(
    const std::string& codec
) const
{
    if (codec == "h264")
    {
        return "nvh264enc";
    }

    Logger::error(
        "Unsupported codec for NVIDIA encoder: " + codec
    );

    return "";
}