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

    auto next() -> float { return XoshiroCpp::FloatFromBits(xoroshiro()); }

    XoshiroCpp::Xoshiro128Plus xoroshiro;

    friend auto cloneForThread(PRNG const &prng, std::size_t threadK) -> PRNG;
};

inline auto cloneForThread(PRNG const &prng, std::size_t threadK) -> PRNG {
    PRNG copy(prng);
    for (decltype(threadK) i = 0; i < threadK; i++)
        copy.xoroshiro.jump();
    return copy;
}
} // namespace cornelis
