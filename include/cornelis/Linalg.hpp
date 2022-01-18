#pragma once

#include <ostream>
#include <tuple>
#include <type_traits>

#include <vector>

#include <cornelis/Expects.hpp>
#include <cornelis/SoA.hpp>
#include <cornelis/Span.hpp>

namespace cornelis {

/**
 * Left multiplication of the column vector x with the matrix A: A.x
 *
 * Result is stored in y. We allow for &y == &x.
 */
auto matrixMultiply(float4x4 const &A, float4 const &x, float4 &y) noexcept -> void;

/**
 * Transforms every ray using the given matrix A.
 *
 * The rays are transformed in place.
 *
 * \note Beware: there is no special consideration taken to the members of ray4, so a ray4 r with
 * r.dir[3] == 1 will get a translated normal.
 */
//auto transformRaySpanss(float4x4 const &A, span<ray4> rays) noexcept -> void;

/**
 * Tests a stream of rays against a single sphere with the given transform and radius.
 *
 * The transform should be ordered in a column fashion.
 *
 * The nearest hit point for ray index k is stored in hits[k]. If a ray misses, hits[k] = INFINITY.
 *
 * Returns true if any ray hit.
 */
/*auto raysSphereTest(ray_spans const &rays,
                    float4x4 const &transform,
                    float radius,
                    span &hits) noexcept -> bool;*/
} // namespace cornelis
