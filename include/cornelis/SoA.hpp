#pragma once

#include <concepts>
#include <ostream>
#include <tuple>
#include <type_traits>

#include <vector>

#include <cornelis/Expects.hpp>
#include <cornelis/Math.hpp>
#include <cornelis/Span.hpp>

namespace cornelis {

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

namespace tags {
struct PositionX {
    using element_type = float;
};

struct PositionY {
    using element_type = float;
};

struct PositionZ {
    using element_type = float;
};

struct DirectionX {
    using element_type = float;
};

struct DirectionY {
    using element_type = float;
};

struct DirectionZ {
    using element_type = float;
};

struct NormalX {
    using element_type = float;
};

struct NormalY {
    using element_type = float;
};

struct NormalZ {
    using element_type = float;
};

struct WidthF {
    using element_type = float;
};

struct HeightF {
    using element_type = float;
};

struct RayParam0 {
    using element_type = float;
};

struct Intersected {
    using element_type = unsigned char; // Note: not bool because of stupid std::vector<bool>.
};

struct MaterialId {
    using element_type = std::size_t; 
};

} // namespace tags

/**
 * Describes a field and it's underlying type. Is used for static reflection.
 */
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
 * SoA means "Structure of Arrays", and refers to storing data for objects in arrays containing
 * field data instead of an array of structs. This is how you should think about this type and
 * any child object as well.
 *
 * The tags are simple marker objects that specify a field. There's two main limitations to this
 * simple model:
 *  - as typedefs are not unique types in C++, we must create a new type for each type of field
 *    even though they might have the same datatype.
 *  - if we have for example a Position field (a 3D-vector) for our objects, and a Direction
 *    field, also a 3D vector, we need 6 different tags to store this.
 *
 * It is believed most of these issues can be alleviated with some helper types.
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


using SoATuple3f = std::tuple<span<float>, span<float>, span<float>>;

// TODO: rename to getPositionSpans or something.
// TODO: this can't deal with const SoAobjects...
template <FieldTag... Fields>
inline auto getPositions(SoAObject<Fields...> &obj) -> decltype(auto) {
    return std::make_tuple(obj.template get<tags::PositionX>(),
                           obj.template get<tags::PositionY>(),
                           obj.template get<tags::PositionZ>());
}

template <FieldTag... Fields>
inline auto getNormalSpans(SoAObject<Fields...> &obj) -> decltype(auto) {
    return std::make_tuple(obj.template get<tags::NormalX>(),
                           obj.template get<tags::NormalY>(),
                           obj.template get<tags::NormalZ>());
}

template <FieldTag... Fields>
inline auto getDirectionSpans(SoAObject<Fields...> &obj) -> decltype(auto) {
    return std::make_tuple(obj.template get<tags::DirectionX>(),
                           obj.template get<tags::DirectionY>(),
                           obj.template get<tags::DirectionZ>());
}

template <FieldTag... Fields>
inline auto setPosition(SoAObject<Fields...> &obj, std::size_t k, float3 P) {
    auto [x, y, z] = getPositions(obj);
    auto [Px, Py, Pz] = P;
    x[k] = Px;
    y[k] = Py;
    z[k] = Pz;
}

template <FieldTag... Fields>
inline auto setDirection(SoAObject<Fields...> &obj, std::size_t k, float3 D) {
    auto [x, y, z] = getDirectionSpans(obj);
    auto [Dx, Dy, Dz] = D;
    x[k] = Dx;
    y[k] = Dy;
    z[k] = Dz;
}

template <FieldTag... Fields>
inline auto setNormal(SoAObject<Fields...> &obj, std::size_t k, float3 N) {
    auto [Nx, Ny, Nz] = getNormalSpans(obj);
    auto [Nx2, Ny2, Nz2] = N;
    Nx[k] = Nx2;
    Ny[k] = Ny2;
    Nz[k] = Nz2;
}

} // namespace cornelis
