#pragma once

#include <vector>
#include <stdint.h>

namespace cornelis {
namespace random {
/**
 * Sets the seed of the global PRNG.
 *
 * This function is safe for multithreaded use.
 *
 * \note After a call to this with a *different* seed than the previous call, this will reseed all
 * the thread local PRNGs. This is however done on the next call to a random number generating
 * function, so this might not be immediately visible on all threads, especially if a race occurs
 * between this and a call to a random number generator.
 */
auto setSeed(uint64_t seed) -> void;


// TODO: use a std::span here.
/**
 * Generates numbers.size() many pseudorandom numbers in [0, 1) using a uniform distribution.
 *
 * This function is safe for multithreaded use but not reentrant. Different threads will start at different points at
 * the PRNG's period.
 */
auto uniform01(std::vector<float> &numbers) -> void;

} // namespace random
} // namespace cornelis
