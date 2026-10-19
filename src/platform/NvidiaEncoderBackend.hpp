#pragma once

#include "EncoderBackend.hpp"

class NvidiaEncoderBackend : public EncoderBackend
{
public:
    std::string getEncoderElement(
        const std::string& codec
    ) const override;
};