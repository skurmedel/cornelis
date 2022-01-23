#pragma once

#include <cornelis/Math.hpp>
#include <cornelis/SoA.hpp>

namespace cornelis {
struct IntersectionData : public SoAObject<tags::Intersected,
                                           tags::RayParam0,
                                           tags::PositionX,
                                           tags::PositionY,
                                           tags::PositionZ,
                                           tags::NormalX,
                                           tags::NormalY,
                                           tags::NormalZ,
                                           tags::MaterialId> {
    IntersectionData(std::size_t n);

    auto reset() -> void;
};

/**
 * Checks for intersections between rays stored in rayOrigins and rayDirs and a sphere.
 *
 * Naturally, rayOrigins, rayDirs and the spans of IntersectionData must share a mutual size
 * (strictly, all spans must be greater or equal to rayOrigins x-span's size).
 *
 * Only iterates over active rays.
 */
auto intersectSphere(SoATuple3f rayOrigins,
                     SoATuple3f rayDirs,
                     float3 sphereCenter,
                     float sphereRadius,
                     std::size_t materialId,
                     IntersectionData &data,
                     std::vector<std::size_t> const &activeRayIds) -> void;

auto intersectPlane(SoATuple3f rayOrigins,
                     SoATuple3f rayDirs,
                     float3 planeNormal,
                     float3 planePoint,
                     float width,
                     float height,
                     std::size_t materialId,
                     IntersectionData &data,
                     std::vector<std::size_t> const &activeRayIds) -> void;

} // namespace cornelis
