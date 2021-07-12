#pragma once

#include <cornelis/NanoVDBMath.hpp>

#include <ostream>

namespace cornelis {
using V3 = nanovdb::Vec3<float>;
using V4 = nanovdb::Vec4<float>;
using BBox = nanovdb::BBox<V3>;
using Ray = nanovdb::Ray<float>;

struct PixelCoord {
    using value_type = int32_t;
    using ValueType = value_type;

    value_type i, j;
};

using BBoxi = nanovdb::BBox<PixelCoord>;

} // namespace cornelis
