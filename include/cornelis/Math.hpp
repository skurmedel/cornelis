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
 * Represents a point in some space (doesn't have to be a vector space).
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
} // namespace std