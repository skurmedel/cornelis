#include <cornelis/Camera.hpp>

#include "MathHelpers.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating.hpp>

using namespace cornelis;

TEST_CASE("horizontalFov35mm") {
    CHECK_THROWS(horizontalFov35mm(0.0f));
    CHECK_THROWS(horizontalFov35mm(-1.0f));

    CHECK_THAT(horizontalFov35mm(50), Catch::Matchers::WithinAbs(0.691111, 0.001));
    CHECK_THAT(horizontalFov35mm(75), Catch::Matchers::WithinAbs(0.47109, 0.001));
}

TEST_CASE("PerspectiveCamera: constructor") {
    PerspectiveCamera cam;

    auto ray = cam(0.5, 0.5);
    CHECK(ray.eye() == V3(0));
    CHECK(ray.dir() == V3(0, 0, 1));
}

TEST_CASE("PerspectiveCamera: lookAt") {
    PerspectiveCamera cam = PerspectiveCamera::lookAt(V3(0), V3(1, 0, 0), 1.0, 1.0);

    auto ray = cam(0.0, 0.5);
    CHECK(ray.eye() == V3(0));
    CHECK(ray.dir() == V3(1.0, 0, 0.4794255386).normalize());

    cam = PerspectiveCamera::lookAt(V3(0, 0, 2), V3(0, 0, 0), 1.0, 1.0);
    ray = cam(0.5, 0.5);
    CHECK(ray.eye() == V3(0, 0, 2));
    CHECK(ray.dir() == V3(0, 0, -1).normalize());
}