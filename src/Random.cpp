#include <atomic>
#include <thread>

#include <cornelis/Random.hpp>

#include "extern/XoshiroCpp.hpp"

using seed_type = uint64_t;
struct PRNG {
    static constexpr uint64_t DefaultSeed = 19791102;

    PRNG(uint64_t s = DefaultSeed) : xoroshiro(s) {}

    auto next() -> double { return XoshiroCpp::DoubleFromBits(xoroshiro()); }

    XoshiroCpp::Xoroshiro128PlusPlus xoroshiro;
};

} // namespace cornelis::random
