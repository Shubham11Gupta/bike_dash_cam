#pragma once

#include "EncoderBackend.hpp"

class SoftwareEncoderBackend : public EncoderBackend
{
public:
    std::string getEncoderElement(
        const std::string& codec) const override;
};