#include "SoftwareEncoderBackend.hpp"

#include <iostream>

std::string SoftwareEncoderBackend::getEncoderElement(
    const std::string& codec) const
{
    if (codec == "h264")
    {
        return "x264enc key-int-max=30";
    }

    std::cerr << "Unsupported codec: "
              << codec
              << std::endl;

    return "";
}