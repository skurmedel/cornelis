#pragma once

#include <ostream>
#include <span>
#include <type_traits>

namespace cornelis {
struct float4 {
    alignas(16) float values[4];

    auto operator[](std::size_t k) noexcept -> float & { return values[k]; }
    auto operator[](std::size_t k) const noexcept -> float const & { return values[k]; }

    static auto point3(float x, float y, float z) noexcept -> float4 {
        return float4{{x, y, z, 1.0f}};
    }

    static auto normal3(float x, float y, float z) noexcept -> float4 {
        return float4{{x, y, z, 0.0f}};
    }

    auto operator==(float4 const &other) const noexcept -> bool {
        bool res[4];
        for (std::size_t k = 0; k < 4; k++)
            res[k] = values[k] == other.values[k];
        return res[0] && res[1] && res[2] && res[3];
    }
    auto operator!=(float4 const &other) const noexcept -> bool { return !(*this == other); }
};
static_assert(std::is_trivial_v<float4>, "float4 should be trivial.");

inline auto operator<<(std::ostream &s, float4 const &f4) -> std::ostream & {
    s << "{" << f4[0] << ", " << f4[1] << ", " << f4[2] << ", " << f4[3] << "}";
    return s;
}

struct float4x4 {
    alignas(16) float values[16];

    static auto identityMatrix() noexcept -> float4x4 {
        // TODO: we should store this as columns, it will generally make Matrix-Vector
        // multiplication much faster and Matrix-Vector multiplication will be done a lot in a ray
        // tracer (where objects are allowed transforms), many times for each ray, possibly
        // multiplied by the number of elements...
        return {{1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f}};
    }

    static auto scalingMatrix(float4 const &diagonal) noexcept -> float4x4 {
        // TODO: see comment above about columns.
        return {{diagonal[0],
                 0.f,
                 0.f,
                 0.f,
                 0.f,
                 diagonal[1],
                 0.f,
                 0.f,
                 0.f,
                 0.f,
                 diagonal[2],
                 0.f,
                 0.f,
                 0.f,
                 0.f,
                 diagonal[3]}};
    }
};
static_assert(std::is_trivial_v<float4x4>, "float4x4 should be trivial.");

struct ray4 {
    ray4() = default;
    /**
     * Constructs a ray with the given position and direction.
     *
     * pos[3] is forced to 1, and dir[3] is forced to 0.
     */
    ray4(float4 const &p, float4 const &d) : pos(p), dir(d) {
        pos[3] = 1.0f;
        dir[3] = 0.0f;
    }

    float4 pos{0, 0, 0, 1};
    float4 dir{0, 0, 0, 0};
};
static_assert(std::is_trivially_copyable_v<ray4>, "ray4 should be trivially copyable.");
static_assert(std::is_trivially_move_constructible_v<ray4>,
              "ray4 should be trivially move constructible.");

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
auto transformRays(float4x4 const &A, std::span<ray4> rays) noexcept -> void;


} // namespace cornelis
