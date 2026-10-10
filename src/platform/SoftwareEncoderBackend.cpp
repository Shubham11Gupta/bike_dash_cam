#include "SoftwareEncoderBackend.hpp"

#include "../logging/Logger.hpp"

std::string SoftwareEncoderBackend::getEncoderElement(
    const std::string& codec) const
{
    if (codec == "h264")
    {
        return "x264enc key-int-max=30";
    }

    Logger::error(
        "Unsupported codec: " + codec
    );

    return "";
}