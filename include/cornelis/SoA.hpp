#pragma once

#include <concepts>
#include <ostream>
#include <tuple>
#include <type_traits>

#include <vector>

#include <cornelis/Expects.hpp>
#include <cornelis/Span.hpp>

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

template <typename... TArgs>
using span_tuple = std::tuple<span<TArgs>...>;

using vec4_spans = span_tuple<float, float, float, float>;
using mat44_spans = span_tuple<float4, float4, float4, float4>;
using ray_spans = span_tuple<float4, float4>;

template <typename T>
using soa_vector_type = std::vector<T>;

struct PositionTag {
    using element_type = float4;
};

struct DirectionTag {
    using element_type = float4;
};

struct TransformTag {
    using element_type = float4x4;
};

struct IndexU64Tag {
    using element_type = int64_t;
};

template <typename T>
concept FieldTag = requires(T tag) {
    std::semiregular<typename T::element_type>;
};

template <FieldTag Tag>
struct FieldVector {
    using tag = Tag;
    using element_type = typename Tag::element_type;

    FieldVector() = default;
    FieldVector(std::size_t n) : elements(n) {}

    std::vector<element_type> elements;
};

/**
 * Stores a collection of fields as vectors of the same length. This allows easy representation of
 * an object as a structure of arrays, while also allowing some static reflection and such.
 *
 * If a given vector needs to be manipulated for some very specific reason, you can get them by
 * casting this to the FieldVector<FieldTag> you are interested in, but be mindful that resizing
 * this vector independently of the others likely breaks the invariants of this object.
 *
 * The primary invariant is this:
 *  all the vectors are of the same size.
 */
template <FieldTag... Fields>
struct SoAObject : public FieldVector<Fields>... {
    using field_tuple = std::tuple<Fields...>;

    SoAObject() = default;
    SoAObject(std::size_t n) : FieldVector<Fields>(n)... {}

    /**
     * Retrieves a span for the given field.
     * 
     * \note Exclusive reads thread safe, but the span returned is most likely not.
     */
    template <FieldTag F>
    auto get() -> span<typename F::element_type> {
        return this->FieldVector<F>::elements;
    }

    /**
     * Resizes all the field vectors belonging to this object to n size.
     *
     * Functionally the same as iterating over each field vector and calling resize(n) individually.
     * 
     * \note If resizing one of the vectors fails, t
     * \note Not thread safe.
     */
    /*auto resizeAll(std::size_t n) -> void {
        // Assumes the size invariant hasn't been invalidated.
        std::size currentN = get<std::tuple_element<0, field_tuple>>().size();
        try {

        }
    }*/
};

} // namespace cornelis
