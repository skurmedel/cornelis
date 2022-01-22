#include <cornelis/Expects.hpp>
#include <cornelis/Geometry.hpp>

namespace cornelis {
IntersectionData::IntersectionData(std::size_t n) : SoAObject(n) { reset(); }

auto IntersectionData::reset() -> void {
    auto rayparams = get<tags::RayParam0>();
    auto intersected = get<tags::Intersected>();
    std::fill(std::begin(rayparams), std::end(rayparams), INFINITY);
    // std::fill(std::begin(intersected), std::end(intersected), false);
}

namespace {
// Compute the dot product between a vector and vector k stored in the SoA spans in vec.
auto dot(float3 v1, std::size_t k, SoATuple3f vec) -> float {
    auto [x, y, z] = vec;
    return cornelis::dot(v1, {x[k], y[k], z[k]});
}
// Compute the dot product between vector k1 and vector k2 stored in the SoA spans in vec.
auto dot(std::size_t k1, std::size_t k2, SoATuple3f vec) -> float {
    auto [x, y, z] = vec;
    return cornelis::dot({x[k1], y[k1], z[k1]}, {x[k2], y[k2], z[k2]});
}

inline auto rayT(SoATuple3f rayOrigins, SoATuple3f rayDirections, std::size_t k, float t)
    -> float3 {
    auto [x, y, z] = rayOrigins;
    auto [Dx, Dy, Dz] = rayDirections;
    return cornelis::rayT({x[k], y[k], z[k]}, {Dx[k], Dy[k], Dz[k]}, t);
}
} // namespace

auto intersectSphere(SoATuple3f rayOrigins,
                     SoATuple3f rayDirs,
                     float3 sphereCenter,
                     float sphereRadius,
                     std::size_t materialId,
                     IntersectionData &data,
                     std::vector<std::size_t> const &activeRayIds) -> void {
    auto [rx, ry, rz] = rayOrigins;
    auto [rdx, rdy, rdz] = rayDirs;
    auto intersected = data.get<tags::Intersected>();
    auto params = data.get<tags::RayParam0>();
    auto materialIds = data.get<tags::MaterialId>();
    auto [IPx, IPy, IPz] = getPositions(data);
    auto [INx, INy, INz] = getNormalSpans(data);
    // TODO: precondition that ensure sizes are the same!

    for (auto k : activeRayIds) {
        /*
            o := ray origin, d := ray direction
            c := sphere center, r := sphere radius
            | o + t * d - c |                            = r      (on the surface for a solution t.)
             ((o - c) + t * d)^2                         = r^2    (left hand size here is a dot)
             (o - c)^2 + 2 * t * (o - c) . d + t^2 * d^2 = r^2    (by distributivity of dot product)

            Let C := (o - c)^2, B := (o - c) . d and A := d^2, then we need to solve:
                A t^2 + 2 B t + C = r^2

            Note d . d = 0  iff. d = (0, 0, 0), which would be be a bogus ray, so we assume d . d !=
           0, so we can simplify to:

                t^2 + 2 B t / A + C / A = r^2 / A
        */
        // Check for a well behaved ray.
        if (rdx[k] == 0 && rdy[k] == 0 && rdz[k] == 0) // TODO: better test PLZ FOR THE LOVE OF GOD.
        {
            intersected[k] = false;
            continue;
        }

        float3 P = float3{rx[k], ry[k], rz[k]} - sphereCenter;

        // If we assumed the direction was normalized we could remove this step.
        // That is: A = 1
        float A = dot(k, k, rayDirs);
        float B = dot(P, k, rayDirs);
        float C = mag2(P);

        float u = 2.0f * B / A;
        float v = (C - sphereRadius * sphereRadius) / A;

        // Solve t^2 + u t + v = 0  <=> (t + u/2)^2 - u^2/4 + v = 0
        float discriminant = -v + (u * u) / 4.0f;
        if (discriminant < 0.0f) {
            intersected[k] = false;
        } else {
            float shift = sqrt(discriminant);
            float t0 = -u / 2.0f - shift;
            float t1 = -u / 2.0f + shift;
            if (t0 < 0.0f)
                t0 = INFINITY;
            if (t1 < 0.0f)
                t1 = INFINITY;
            float t = t0 < t1 ? t0 : t1;
            // Only update if we hit closer than previous data.
            if (params[k] > t) {
                params[k] = t;
                intersected[k] = true;
                float3 sP = rayT(rayOrigins, rayDirs, k, t);
                setPosition(data, k, sP);
                setNormal(data, k, normalize(sP - sphereCenter));
                materialIds[k] = materialId;
            }
        }
    }
}

auto intersectPlane(SoATuple3f rayOrigins,
                    SoATuple3f rayDirs,
                    float3 planeNormal,
                    float3 planePoint,
                    std::size_t materialId,
                    IntersectionData &data,
                    std::vector<std::size_t> const &activeRayIds) -> void {
    auto [rx, ry, rz] = rayOrigins;
    auto [rdx, rdy, rdz] = rayDirs;
    auto intersected = data.get<tags::Intersected>();
    auto params = data.get<tags::RayParam0>();
    auto materialIds = data.get<tags::MaterialId>();
    auto [IPx, IPy, IPz] = getPositions(data);
    auto [INx, INy, INz] = getNormalSpans(data);
    // TODO: precondition that ensure sizes are the same!

    for (auto k : activeRayIds) {
        /*
            A point c is in the plane if (c - P) . N = 0 where P a point on the plane, N its normal.

            Substituting the ray equation:

                (o + t d - P) . N = ( (o - P) + t d ) . N = (o - P) . N + t d . N = 0

            And so

                t = -((o - P) . N) / d . N

            We need to check for d . N != 0. If d . N and o == P, t = 0, otherwise there is no
           solution.

            We'll call A := -((o - P) . N), B := d . N
        */
        // Check for a well behaved ray.
        if (isAlmostZero(rdx[k]) && isAlmostZero(rdy[k]) && isAlmostZero(rdz[k])) {
            intersected[k] = false;
            continue;
        }

        float3 diff = float3{rx[k], ry[k], rz[k]} - planePoint;
        float A = -dot(diff, planeNormal);
        float B = dot(float3{rdx[k], rdy[k], rdz[k]}, planeNormal);

        if (diff != float3{0} && isAlmostZero(B)) // TODO: BETTER TEST!
            intersected[k] = false;
        else {
            float t = 0.0f;
            if (!isAlmostZero(B))
                t = A / B;
            if (params[k] > t) {
                params[k] = t;
                intersected[k] = true;
                float3 sP = rayT(rayOrigins, rayDirs, k, t);
                setPosition(data, k, sP);
                setNormal(data, k, normalize(planeNormal));
                materialIds[k] = materialId;
            }
        }
    }
}

} // namespace cornelis
