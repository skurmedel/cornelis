#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating.hpp>

#include <cornelis/Geometry.hpp>
#include <cornelis/SoA.hpp>

using namespace cornelis;

namespace {
struct TestRays : SoAObject<tags::PositionX,
                            tags::PositionY,
                            tags::PositionZ,
                            tags::DirectionX,
                            tags::DirectionY,
                            tags::DirectionZ> {
    TestRays(std::size_t n) : SoAObject(n) {}
};
} // namespace

TEST_CASE("intersectSphere") {
    TestRays rays(6);

    auto [rx, ry, rz] = getPositions(rays);
    auto [dirx, diry, dirz] = getDirectionSpans(rays);
    std::vector<std::size_t> activeRayIds = {0, 1, 2, 3, 5};

    IntersectionData intersections(rx.size());
    auto params = intersections.get<tags::RayParam0>();
    auto intersected = intersections.get<tags::Intersected>();

    // TODO: test that we do not intersect "backwards".
    // Ray 1 (2 intersections)
    rx[0] = -1.5f;
    ry[0] = 0;
    rz[0] = -3.0f;
    dirx[0] = 0;
    diry[0] = 0;
    dirz[0] = 1.0f;

    // Ray 2 (1 intersection; double root)
    rx[1] = -2;
    ry[1] = 0;
    rz[1] = -3.0f;
    dirx[1] = 0;
    diry[1] = 0;
    dirz[1] = 2.0f; // Let's have the direction non-normalized.

    // Ray 3 (miss)
    rx[2] = 0;
    ry[2] = 2.0f;
    rz[2] = -3.0f;
    dirx[2] = 0;
    diry[2] = 0;
    dirz[2] = 1.0f;

    // Ray 4 (bad ray)
    rx[3] = 0;
    ry[3] = 0.0f;
    rz[3] = -3.0f;
    dirx[3] = 0;
    diry[3] = 0;
    dirz[3] = 0;

    // Ray 5 (would hit but is not active)
    rx[4] = -1;
    ry[4] = 0;
    rz[4] = -3.0f;
    dirx[4] = 0;
    diry[4] = 0;
    dirz[4] = 1.0f;

    // Ray 6 (would hit but previous t0 is closer)
    rx[5] = -1;
    ry[5] = 0;
    rz[5] = -3.0f;
    dirx[5] = 0;
    diry[5] = 0;
    dirz[5] = 1.0f;
    params[5] = -0.5f;
    intersected[5] = true;

    std::size_t materialId = 42;
    intersectSphere(getPositions(rays),
                    getDirectionSpans(rays),
                    {-1, 0, 0},
                    1.0f,
                    materialId,
                    intersections,
                    activeRayIds);

    auto [x, y, z] = getPositions(intersections);
    auto [Nx, Ny, Nz] = getNormalSpans(intersections);
    auto materialIds = intersections.get<tags::MaterialId>();

    CAPTURE(x[0], y[0], z[0], x[1], y[1], z[1], Nx[0], Ny[0], Nz[0]);

    CHECK(intersected[0] == true);
    CHECK_THAT(x[0], Catch::Matchers::WithinAbs(-1.5f, 0.001f));
    CHECK_THAT(y[0], Catch::Matchers::WithinAbs(0, 0.001f));
    CHECK_THAT(z[0], Catch::Matchers::WithinAbs(-0.86603f, 0.001f));

    CHECK_THAT(Nx[0], Catch::Matchers::WithinAbs(-0.5f, 0.001f));
    CHECK_THAT(Ny[0], Catch::Matchers::WithinAbs(0, 0.001f));
    CHECK_THAT(Nz[0], Catch::Matchers::WithinAbs(-0.86601f, 0.001f));

    // CHECK((Nx[0] == -0.94491 && Ny[0] == 0 && Nz[0] == -0.32731));
    CHECK_THAT(params[0], Catch::Matchers::WithinAbs(2.1339f, 0.001f));
    CHECK(materialIds[0] == materialId);

    CHECK(intersected[1] == true);
    CHECK((x[1] == -2.0f && y[1] == 0 && z[1] == 0));
    CHECK((Nx[1] == -1.0f && Ny[1] == 0 && Nz[1] == 0));
    CHECK(params[1] == 1.5f);
    CHECK(materialIds[1] == materialId);

    CHECK(intersected[2] == false);

    CHECK(intersected[3] == false);

    CHECK(intersected[4] == false);

    CHECK(intersected[5] == true);
    CHECK(params[5] == -0.5f);
}

TEST_CASE("intersectPlane") {
    TestRays rays(6);

    auto [rx, ry, rz] = getPositions(rays);
    auto [dirx, diry, dirz] = getDirectionSpans(rays);
    std::vector<std::size_t> activeRayIds = {0, 1, 2, 3, 5};

    std::size_t materialId = 63;
    float3 planeN = normalize(float3{1, 0, -1});
    float3 planeP{-1.0, 0, 0};
    float planeWidth = 100.0f;
    float planeHeight = 50.0f;

    IntersectionData intersections(rx.size());
    auto params = intersections.get<tags::RayParam0>();
    auto intersected = intersections.get<tags::Intersected>();

    auto [x, y, z] = getPositions(intersections);
    auto [Nx, Ny, Nz] = getNormalSpans(intersections);
    auto materialIds = intersections.get<tags::MaterialId>();

    // TODO: test that we do not intersect "backwards".
    // Ray 1 (1 intersections)
    rx[0] = -1.5f;
    ry[0] = 0;
    rz[0] = -3.0f;
    dirx[0] = 0;
    diry[0] = 0;
    dirz[0] = 1.0f;

    // Ray 2 (infinite intersections, lies in the plane)
    rx[1] = -1.0;
    ry[1] = 0;
    rz[1] = 0;
    dirx[1] = 1;
    diry[1] = 0;
    dirz[1] = 1; // Let's have the direction non-normalized.

    // Ray 3 (miss, starts outside, parallel with the plane)
    rx[2] = 0.0;
    ry[2] = 0.0;
    rz[2] = 0.0;
    dirx[2] = 1.0f;
    diry[2] = 0;
    dirz[2] = 1.0f;

    // Ray 4 (bad ray)
    rx[3] = 0;
    ry[3] = 0.0f;
    rz[3] = -3.0f;
    dirx[3] = 0;
    diry[3] = 0;
    dirz[3] = 0;

    // Ray 5 (would hit but is not active)
    rx[4] = -1.5f;
    ry[4] = 0;
    rz[4] = -3.0f;
    dirx[4] = 0;
    diry[4] = 0;
    dirz[4] = 1.0f;

    // Ray 6 (would hit but previous t0 is closer)
    rx[5] = -1.5f;
    ry[5] = 0;
    rz[5] = -3.0f;
    dirx[5] = 0;
    diry[5] = 0;
    dirz[5] = 1.0f;
    params[5] = -0.5f;
    intersected[5] = true;

    intersectPlane(getPositions(rays),
                   getDirectionSpans(rays),
                   planeN,
                   planeP,
                   planeWidth,
                   planeHeight,
                   materialId,
                   intersections,
                   activeRayIds);

    CAPTURE(x[0], y[0], z[0], x[1], y[1], z[1], Nx[0], Ny[0], Nz[0]);

    CHECK(intersected[0] == true);
    CHECK_THAT(x[0], Catch::Matchers::WithinAbs(-1.5f, 0.001f));
    CHECK_THAT(y[0], Catch::Matchers::WithinAbs(0, 0.001f));
    CHECK_THAT(z[0], Catch::Matchers::WithinAbs(-0.5f, 0.001f));

    CHECK_THAT(Nx[0], Catch::Matchers::WithinAbs(planeN(0), 0.001f));
    CHECK_THAT(Ny[0], Catch::Matchers::WithinAbs(planeN(1), 0.001f));
    CHECK_THAT(Nz[0], Catch::Matchers::WithinAbs(planeN(2), 0.001f));

    // CHECK((Nx[0] == -0.94491 && Ny[0] == 0 && Nz[0] == -0.32731));
    CHECK_THAT(params[0], Catch::Matchers::WithinAbs(2.5f, 0.001f));
    CHECK(materialIds[0] == materialId);

    CHECK(intersected[1] == true);
    CHECK((x[1] == -1.0f && y[1] == 0 && z[1] == 0));
    CHECK_THAT(Nx[1], Catch::Matchers::WithinAbs(planeN(0), 0.001f));
    CHECK_THAT(Ny[1], Catch::Matchers::WithinAbs(planeN(1), 0.001f));
    CHECK_THAT(Nz[1], Catch::Matchers::WithinAbs(planeN(2), 0.001f));
    CHECK(params[1] == 0.0f);
    CHECK(materialIds[1] == materialId);

    CHECK(intersected[2] == false);

    CHECK(intersected[3] == false);

    CHECK(intersected[4] == false);

    CHECK(intersected[5] == true);
    CHECK(params[5] == -0.5f);
}