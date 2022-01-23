#include <cornelis/Scene.hpp>

#include "MathHelpers.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating.hpp>

using namespace cornelis;

TEST_CASE("SphereData: constructor") {
    std::vector<SphereDescription> descriptions;
    SphereData data1(descriptions);

    {
        auto [x, y, z] = getPositions(data1);
        CHECK(x.size() == 0);
    }

    SphereDescription sphere1 = {.center = V3(0.5, 2.0, 4.0), .radius = 2.0f};
    sphere1.material = 3;
    descriptions.push_back(sphere1);

    SphereData data2(descriptions);

    {
        auto [x, y, z] = getPositions(data2);
        CHECK(x[0] == sphere1.center[0]);
        CHECK(y[0] == sphere1.center[1]);
        CHECK(z[0] == sphere1.center[2]);
        auto radius = data2.get<tags::Radius>();
        CHECK(radius[0] == sphere1.radius);
        auto matid = data2.get<tags::MaterialId>();
        CHECK(matid[0] == sphere1.material);
    }
}

TEST_CASE("PlaneData: constructor") {
    std::vector<PlaneDescription> descriptions;
    PlaneData data1(descriptions);

    {
        auto [x, y, z] = getPositions(data1);
        CHECK(x.size() == 0);
        auto [Nx, Ny, Nz] = getPositions(data1);
        CHECK(Nx.size() == 0);
    }

    PlaneDescription plane1 = { .normal = V3(1), .point = V3(0.5, 2.0, 4.0)};
    plane1.material = 3;
    descriptions.push_back(plane1);

    PlaneData data2(descriptions);

    {
        auto [x, y, z] = getPositions(data2);
        CHECK(x[0] == plane1.point[0]);
        CHECK(y[0] == plane1.point[1]);
        CHECK(z[0] == plane1.point[2]);
        
        auto [Nx, Ny, Nz] = getNormalSpans(data2);
        CHECK(Nx[0] == plane1.normal[0]);
        CHECK(Ny[0] == plane1.normal[1]);
        CHECK(Nz[0] == plane1.normal[2]);

        auto matid = data2.get<tags::MaterialId>();
        CHECK(matid[0] == plane1.material);
    }
}