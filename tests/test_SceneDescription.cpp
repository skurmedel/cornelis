#include <cornelis/SceneDescription.hpp>

#include "MathHelpers.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating.hpp>

using namespace cornelis;

TEST_CASE("SceneDescription::constructor") {
    SceneDescription descr;
    // Check for a default material at position 0.
    CHECK(descr.materials().size() == 1);
}

TEST_CASE("SceneDescription::addMaterial") {
    SceneDescription descr;
    MaterialDescription mat{.albedo = RGB::red()};
    descr.addMaterial(mat);

    CHECK(descr.materials().size() == 2);
    CHECK(descr.materials()[1] == mat);
}

TEST_CASE("SceneDescription::addSphere") {
    SceneDescription descr;
    SphereDescription sphere{.center = V3(2.3), .radius = 5.0f};
    descr.addSphere(sphere);

    CHECK(descr.spheres().size() == 1);
    CHECK(descr.spheres()[0] == sphere);
}

TEST_CASE("SceneDescription::addPlane") {
    SceneDescription descr;
    PlaneDescription plane{.normal = V3(2.3), .point = V3(1)};
    descr.addPlane(plane);

    CHECK(descr.planes().size() == 1);
    CHECK(descr.planes()[0] == plane);
}