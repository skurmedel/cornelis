#pragma once

#include <stdint.h>

namespace cornelis {
struct RenderOptions {
    static constexpr int32_t DefaultSamplesAA = 1 << 8;

    /**
     * Number of samples to use for anti-aliasing a pixel. As Cornelis is a Monte-Carlo path tracer,
     * this generally improves all kind of noise.
     *
     * As such, it's the main quality control parameter.
     */
    decltype(DefaultSamplesAA) samplesAA = DefaultSamplesAA;
};
} // namespace cornelis
