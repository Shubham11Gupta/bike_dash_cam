#pragma once

#include <string>

class EncoderBackend
{
public:
    virtual ~EncoderBackend() = default;

    virtual std::string getEncoderElement(
        const std::string& codec) const = 0;
};