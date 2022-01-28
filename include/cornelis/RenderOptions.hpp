#pragma once

#include <stdint.h>

namespace cornelis {
struct RenderOptions {
    static constexpr int32_t DefaultSamplesAA = 1<<8;

    int32_t samplesAA = DefaultSamplesAA;

};
} // namespace cornelis
