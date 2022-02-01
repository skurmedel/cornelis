#pragma once

#include <cornelis/Expects.hpp>
#include <cornelis/NanoVDBMath.hpp>

#include <array>
#include <concepts>
#include <ostream>
#include <tuple>
#include <type_traits>
#include <utility>

namespace cornelis {
using V3 = nanovdb::Vec3<float>;
using V4 = nanovdb::Vec4<float>;
using BBox = nanovdb::BBox<V3>;
using Ray = nanovdb::Ray<float>;

// TODO: find a number for this based on brain power and not guesswork.
inline constexpr float RayEpsilon = 0.00005f;

inline auto isAlmostZero(float v) -> bool { return abs(v) < RayEpsilon; }

// On C++20 we could use <numbers>
constexpr float Pi = 3.14159265359f;

/**
 * This concept is named after the algebraic structure called a product ring,
 * and represents the tuples of the Cartesian product of N rings.
 *
 * We only allow one kind of ring R, be it rational (floating point) or integers.
 *
 * It has component-wise addition and multiplication. If R is rational, we can also view it as a
 * vector although the component-wise multiplication has bad properties in a vector space.
 *
 * All this sounds very fancy, but as a programmer this is mostly a way to let us create tuples
 * with some common operations that are mathematically sound from this point of view.
 *
 * We need tuples of integers for sRGB colors and pixel coordinates. And we need tuples of
 * floating point numbers for pure spectral colors, 3D vectors and so forth.
 */
template <typename P>
concept product_ring = (
    requires {
        std::common_with<decltype(P::N), std::size_t>;
        typename P::element_type;
        std::integral<typename P::element_type> || std::floating_point<typename P::element_type>;
    } &&
    requires(P p, std::size_t i, typename P::element_type s) {
        { p(i) } -> std::same_as<typename P::element_type &>;
        {p(i) + s};
        {p(i) - s};
        {p(i) * s};
    } &&
    requires(P const &pc, std::size_t i) {
        { pc(i) } -> std::same_as<typename P::element_type const &>;
    } &&
    std::default_initializable<P>);

/**
 * Componentwise addition on a product_ring.
 */
template <product_ring P>
inline auto operator+(P const &a, P const &b) noexcept -> P {
    P res;
    for (std::size_t i = 0; i < P::N; i++) {
        res(i) = a(i) + b(i);
    }
    return res;
}

/**
 * Componentwise subtraction on a product_ring.
 */
template <product_ring P>
inline auto operator-(P const &a, P const &b) noexcept -> P {
    P res;
    for (std::size_t i = 0; i < P::N; i++) {
        res(i) = a(i) - b(i);
    }
    return res;
}
/**
 * Componentwise subtraction on a product_ring.
 */
template <product_ring P>
inline auto operator-(P const &a) noexcept -> P {
    P res;
    for (std::size_t i = 0; i < P::N; i++) {
        res(i) = -a(i);
    }
    return res;
}

/**
 * Componentwise multiplication on a product_ring.
 */
template <product_ring P>
inline auto operator*(P const &a, P const &b) noexcept -> P {
    P res;
    for (std::size_t i = 0; i < P::N; i++) {
        res(i) = a(i) * b(i);
    }
    return res;
}

/**
 * Componentwise right-multiplication with element type promoted to full tuple.
 */
template <product_ring P>
inline auto operator*(P const &a, typename P::element_type const &b) noexcept -> P {
    P res;
    for (std::size_t i = 0; i < P::N; i++) {
        res(i) = a(i) * b;
    }
    return res;
}
/**
 * Componentwise left-multiplication with element type promoted to full tuple.
 */
template <product_ring P>
inline auto operator*(typename P::element_type const &a, P const &b) noexcept -> P {
    P res;
    for (std::size_t i = 0; i < P::N; i++) {
        res[i] = a * b[i];
    }
    return res;
}

template <std::size_t idx, product_ring P>
inline auto get(P &p) -> std::tuple_element_t<idx, P> & {
    return p(idx);
}
template <std::size_t idx, product_ring P>
inline auto get(P const &p) -> std::tuple_element_t<idx, P> const & {
    return p(idx);
}

template <std::size_t idx, product_ring P>
inline auto get(P &&p) -> std::tuple_element_t<idx, P> && {
    return std::move(p(idx));
}
template <std::size_t idx, product_ring P>
inline auto get(P const &&p) -> std::tuple_element_t<idx, P> const && {
    return std::move(p(idx));
}

template <typename From, typename To>
concept implicit_convertible = requires {
    To(std::declval<From>());
};

/**
 * A wrapper around an array of N floats. It satisfies various concepts used to provide mathematical
 * operations.
 */
template <std::size_t NumElements>
struct floatN {
    using element_type = float;
    inline static const std::size_t N = NumElements;

    /**
     * Zero initializes the floatN. (It's not worth the bugs to do otherwise)
     */
    floatN() : values{} {};
    /**
     * Fills the float with a specified value.
     */
    explicit floatN(float a) : values{} { values.fill(a); }
    /**
     * Takes n implicit convertible values and sets the members of the float accordingly.
     */
    floatN(implicit_convertible<float> auto... args) requires
        std::bool_constant<sizeof...(args) == NumElements>::value
        : values{static_cast<float>(args)...} {}

    floatN(floatN<N> const &) = default;
    floatN(floatN<N> &&) = default;
    auto operator=(floatN<N> &) -> floatN<N> & = default;
    auto operator=(floatN<N> &&) -> floatN<N> & = default;

    auto operator()(std::size_t i) -> float & { return values[i]; }
    auto operator()(std::size_t i) const -> float const & { return values[i]; }

    auto operator==(floatN<N> const &) const -> bool = default;

    std::array<element_type, N> values;
};

using float2 = floatN<2>;
static_assert(product_ring<float2>);

using float3 = floatN<3>;
static_assert(product_ring<float3>);

// TODO: Make this into a product_ring
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

/**
 * Treats two float3 objects as vectors and computes the dot product.
 */
inline auto dot(float3 a, float3 b) -> float { return a(0) * b(0) + a(1) * b(1) + a(2) * b(2); }

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
    using element_type = float;
    static constexpr std::size_t N = 4;

    auto operator()(std::size_t k) noexcept -> float & { return values[k]; }
    auto operator()(std::size_t k) const noexcept -> float const & { return values[k]; }

    auto operator==(float4 const &) const -> bool = default;

    static auto point3(float x, float y, float z) noexcept -> float4 { return init(x, y, z, 1.0f); }

    static auto normal3(float x, float y, float z) noexcept -> float4 {
        return init(x, y, z, 0.0f);
    }

    static auto init(float c) -> float4 { return float4::init(c, c, c, c); }

    static auto init(float x1, float x2, float x3, float x4) -> float4 {
        return float4{{x1, x2, x3, x4}};
    }

    alignas(16) element_type values[N];
};
static_assert(product_ring<float4>, "float4 should be a product_ring.");

inline auto operator<<(std::ostream &s, float4 const &f4) -> std::ostream & {
    s << "{" << f4(0) << ", " << f4(1) << ", " << f4(2) << ", " << f4(3) << "}";
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
        return {{diagonal(0),
                 0.f,
                 0.f,
                 0.f,
                 0.f,
                 diagonal(1),
                 0.f,
                 0.f,
                 0.f,
                 0.f,
                 diagonal(2),
                 0.f,
                 0.f,
                 0.f,
                 0.f,
                 diagonal(3)}};
    }
};
static_assert(std::is_trivial_v<float4x4>, "float4x4 should be trivial.");

} // namespace cornelis

// Specializations for structured binding support.
namespace std {
template <cornelis::product_ring P>
struct tuple_size<P> : integral_constant<size_t, P::N> {};

template <typename P, std::size_t k>
requires(cornelis::product_ring<P>) struct tuple_element<k, P> {
    using type = typename P::element_type;
};
} // namespace std

namespace cornelis {
/**
 * Interprets v1 and v2 as 3D vectors and computes their cross product.
 */
inline auto cross(float3 v1, float3 v2) -> float3 {
    auto [x1, x2, x3] = v1;
    auto [y1, y2, y3] = v2;
    return {x2 * y3 - x3 * y2, x3 * y1 - x1 * y3, x1 * y2 - x2 * y1};
}

/**
 * Normalize a float3 interpreted as a 3D vector.
 *
 * To avoid floating point issues this function has a step for vectors of a small enough magnitude.
 * Below this cut-off, it treats it as a zero vector.
 */
inline auto normalize(float3 v1) -> float3 {
    auto len = sqrtf(mag2(v1));
    if (isAlmostZero(len))
        return float3{0};
    auto s = 1.0f / len;
    return v1 * float3{s};
}

/**
 * Represents the basis vectors for some 3D coordinate system.
 */
struct Basis {
    /**
     * The normal, commonly imagined as "up" and the Z-axis.
     */
    float3 N;
    /**
     * Tangent vector, likes along the tangent plane.
     */
    float3 T;
    /**
     * Bi-tangent vector, orthonormal to T, but still in the tangent plane.
     */
    float3 B;
};

/**
 * Makes up a basis using N as the normal vector. It's formulated this way because its primary use
 * is to construct tangent planes.
 *
 * \pre N is normalized.
 */
inline auto constructBasis(float3 const &N) -> Basis {
    // Invent a tangent.
    float3 helper(0.0f, 1.0f, 0.0f);
    if (abs(N(1)) > 0.95)
        helper = float3(0.0, 0.0, 1.0);
    Basis base{.N = N};
    base.T = normalize(cross(helper, N));
    base.B = cross(base.T, N);
    return base;
}

} // namespace cornelis