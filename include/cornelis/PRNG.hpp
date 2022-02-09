#pragma once

#include <atomic>
#include <thread>

#include <XoshiroCpp.hpp>

namespace cornelis {
// TODO: clean up this place.
using seed_type = uint64_t;
struct PRNG {
    static constexpr uint64_t DefaultSeed = 19791102;

    PRNG(uint64_t s = DefaultSeed) : xoroshiro(s) {}

    /**
     * Generate a floating point number in the interval [0, 1). The distribution is uniform.
     */
    // TODO: this name sucks, fix it.
    auto next() noexcept -> float { return XoshiroCpp::FloatFromBits(xoroshiro()); }

    /**
     * Alias for next().
     */
    auto operator()() noexcept -> float { return next(); }

    XoshiroCpp::Xoshiro128Plus xoroshiro;

    friend auto cloneForThread(PRNG const &prng, std::size_t threadK) -> PRNG;
};

inline auto cloneForThread(PRNG const &prng, std::size_t threadK) -> PRNG {
    PRNG copy(prng);
    for (decltype(threadK) i = 0; i < threadK; i++)
        copy.xoroshiro.jump();
    return copy;
}

inline auto randomHemisphere(float2 const x) -> float3 {
    auto [x1, x2] = x;

    float a = 2.0 * Pi * x2;
    float b = sqrt(1.0f - x1 * x1);

    return float3(cos(a) * b, sin(a) * b, x1);
}

inline auto randomHemisphere(PRNG &prng) -> float3 {
    return randomHemisphere(float2(prng(), prng()));
}

inline auto randomHemisphere(float2 x, Basis const &base) -> float3 {
    float3 v = randomHemisphere(x);
    return base.B * v(0) + base.T * v(1) + base.N * v(2);
}

inline auto randomHemisphere(PRNG &prng, Basis const &base) -> float3 {
    float3 v = randomHemisphere(prng);
    return base.B * v(0) + base.T * v(1) + base.N * v(2);
}

constexpr auto randomHemispherePDF() -> float { return 1.0f / (2.0f * Pi); }

} // namespace cornelis
