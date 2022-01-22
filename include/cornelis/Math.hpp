#pragma once

#include <cornelis/Expects.hpp>
#include <cornelis/NanoVDBMath.hpp>

#include <ostream>
#include <tuple>
#include <type_traits>

namespace cornelis {
using V3 = nanovdb::Vec3<float>;
using V4 = nanovdb::Vec4<float>;
using BBox = nanovdb::BBox<V3>;
using Ray = nanovdb::Ray<float>;

// On C++20 we could use <numbers>
constexpr float Pi = 3.14159265359f;

/**
 * Represents a point in some 2D space (doesn't have to be a vector space).
 */
template <typename T = float>
struct P2 {
    using element_type = T;
    using value_type = element_type;

    P2() : values{0} {}
    P2(element_type x, element_type y) : values{x, y} {}

    template <std::size_t idx>
    auto get() -> std::tuple_element_t<idx, P2<T>> & {
        return values[idx];
    }
    template <std::size_t idx>
    auto get() const -> std::tuple_element_t<idx, P2<T>> const & {
        return values[idx];
    }

    element_type values[2];
};

// TODO: Make this a P2 instead.
struct PixelCoord {
    using value_type = int32_t;
    using ValueType = value_type;

    value_type i, j;
};

/**
 * Describes a rectangle in pixel coordinates. It is represented by two 2D integer points,
 * inclusively.
 *
 * Formally, let p = (n, m), q = (u, v), where p is the min point, q the max point, then any point
 * (x, y) inside this rect adhere to: n <= x <= u and m <= y <= v
 *
 * Which implies that we cannot represent an empty rectangle, but we accept this deficiency.
 */
class PixelRect {
  public:
    using coord_type = PixelCoord;
    using element_type = coord_type::value_type;
    // TODO: constexpr?

    PixelRect() : p0(), p1() {}
    // TODO: tests for this constructor
    explicit PixelRect(PixelCoord dimensions)
        : PixelRect(PixelCoord{0, 0}, PixelCoord{dimensions.i - 1, dimensions.j - 1}) {
        CORNELIS_EXPECTS(dimensions.i != 0 && dimensions.j != 0,
                         "PixelRect cannot represent lines or the empty rectangle.");
    }
    PixelRect(element_type w, element_type h) : PixelRect(PixelCoord{w, h}) {}
    PixelRect(PixelCoord a, PixelCoord b)
        : p0{std::min(a.i, b.i), std::min(a.j, b.j)}, p1{std::max(a.i, b.i), std::max(a.j, b.j)} {}

    auto width() const noexcept -> element_type { return p1.i - p0.i + 1; }

    auto height() const noexcept -> element_type { return p1.j - p0.j + 1; }

    auto area() const noexcept -> element_type { return width() * height(); }

    // TODO: tests for equality
    auto operator==(PixelRect const &other) const noexcept -> bool {
        return p0.i == other.p0.i && p0.j == other.p0.j && p1.i == other.p1.i && p1.j == other.p1.j;
    }
    auto operator!=(PixelRect const &other) const noexcept -> bool { return !(*this == other); }

    friend inline auto operator<<(std::ostream &s, PixelRect const &rect) -> std::ostream &;

    auto min() const noexcept -> PixelCoord const & { return p0; }

    auto max() const noexcept -> PixelCoord const & { return p1; }

  private:
    PixelCoord p0;
    PixelCoord p1;
};

inline auto operator<<(std::ostream &s, PixelRect const &rect) -> std::ostream & {
    s << "PixelRect{{" << rect.p0.i << ", " << rect.p0.j << "}, {" << rect.p1.i << ", " << rect.p1.j
      << "}}";
    return s;
}

// TODO: make me complete.
struct Transform {};

struct float3 {
    alignas(16) float values[3];

    float3() = default;

    float3(float k) : values{k, k, k} {}
    float3(float x1, float x2, float x3) : values{x1, x2, x3} {}

    auto operator==(float3 const &) const noexcept -> bool = default;

    auto operator[](std::size_t k) noexcept -> float & { return values[k]; }
    auto operator[](std::size_t k) const noexcept -> float const & { return values[k]; }

    template <std::size_t idx>
    auto get() -> std::tuple_element_t<idx, float3> & {
        return values[idx];
    }

    template <std::size_t idx>
    auto get() const -> std::tuple_element_t<idx, float3> const & {
        return values[idx];
    }

    /**
     * Componentwise addition.
     */
    auto operator+(float3 const &other) const noexcept -> float3 {
        float3 res;
        for (auto i = 0; i < 3; i++) {
            res.values[i] = values[i] + other[i];
        }
        return res;
    }

    /**
     * Componentwise subtraction.
     */
    auto operator-(float3 const &other) const noexcept -> float3 {
        float3 res;
        for (auto i = 0; i < 3; i++) {
            res.values[i] = values[i] - other[i];
        }
        return res;
    }

    /**
     * Componentwise negation.
     */
    auto operator-() const noexcept -> float3 {
        float3 res;
        for (auto i = 0; i < 3; i++) {
            res.values[i] = -values[i];
        }
        return res;
    }

    /**
     * Componentwise multiplication.
     */
    auto operator*(float3 const &other) const noexcept -> float3 {
        float3 res;
        for (auto i = 0; i < 3; i++) {
            res.values[i] = values[i] * other[i];
        }
        return res;
    }

    auto operator*(float other) const noexcept -> float3 {
        float3 res;
        for (auto i = 0; i < 3; i++) {
            res.values[i] = values[i] * other;
        }
        return res;
    }
};

/**
 * Treats two float3 objects as vectors and computes the dot product.
 */
inline auto dot(float3 a, float3 b) -> float { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }

/**
 * Treats a float3 object as a vector and computes the magnitude raised to two, i.e the unsquared
 * length.
 */
inline auto mag2(float3 a) -> float { return dot(a, a); }

/**
 * Treats two float3 objects as the ray origin and ray direction and computes the position along
 * the ray for parameter t.
 */
inline auto rayT(float3 origin, float3 dir, float t) -> float3 {
    return origin + dir * float3{t, t, t};
}

/**
 * Represents four float values, packed together.
 *
 * This is not necessarily a vector unless you treat it like one, though it's primary use will be to
 * store vectors.
 */
struct float4 {
    alignas(16) float values[4];

    auto operator[](std::size_t k) noexcept -> float & { return values[k]; }
    auto operator[](std::size_t k) const noexcept -> float const & { return values[k]; }

    static auto point3(float x, float y, float z) noexcept -> float4 { return init(x, y, z, 1.0f); }

    static auto normal3(float x, float y, float z) noexcept -> float4 {
        return init(x, y, z, 0.0f);
    }

    static auto init(float c) -> float4 { return float4::init(c, c, c, c); }

    static auto init(float x1, float x2, float x3, float x4) -> float4 {
        return float4{{x1, x2, x3, x4}};
    }

    auto operator==(float4 const &other) const noexcept -> bool {
        bool res[4];
        for (std::size_t k = 0; k < 4; k++)
            res[k] = values[k] == other.values[k];
        return res[0] && res[1] && res[2] && res[3];
    }
    auto operator!=(float4 const &other) const noexcept -> bool { return !(*this == other); }

    /**
     * Componentwise addition.
     */
    auto operator+(float4 const &other) const noexcept -> float4 {
        float4 res;
        for (auto i = 0; i < 4; i++) {
            res.values[i] = values[i] + other[i];
        }
        return res;
    }

    /**
     * Componentwise subtraction.
     */
    auto operator-(float4 const &other) const noexcept -> float4 {
        float4 res;
        for (auto i = 0; i < 4; i++) {
            res.values[i] = values[i] - other[i];
        }
        return res;
    }

    /**
     * Componentwise multiplication.
     */
    auto operator*(float4 const &other) const noexcept -> float4 {
        float4 res;
        for (auto i = 0; i < 4; i++) {
            res.values[i] = values[i] * other[i];
        }
        return res;
    }
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

} // namespace cornelis

// Specializations for structured binding support.
namespace std {
template <typename T>
struct tuple_size<cornelis::P2<T>> : integral_constant<size_t, 2> {};

template <typename T>
struct tuple_element<0, cornelis::P2<T>> {
    using type = T;
};
template <typename T>
struct tuple_element<1, cornelis::P2<T>> {
    using type = T;
};

template <>
struct tuple_size<cornelis::float3> : integral_constant<size_t, 3> {};

template <>
struct tuple_element<0, cornelis::float3> {
    using type = float;
};
template <>
struct tuple_element<1, cornelis::float3> {
    using type = float;
};
template <>
struct tuple_element<2, cornelis::float3> {
    using type = float;
};
} // namespace std

namespace cornelis {
inline auto cross(float3 v1, float3 v2) -> float3 {
    auto [x1, x2, x3] = v1;
    auto [y1, y2, y3] = v2;
    return {x2 * y3 - x3 * y2, x3 * y1 - x1 * y3, x1 * y2 - x2 * y1};
}

inline auto normalize(float3 v1) -> float3 {
    auto len = sqrtf(mag2(v1));
    if (len == 0) // TODO: better test omg.
        return {0};
    auto s = 1.0f / len;
    return v1 * float3{s};
}
} // namespace cornelis