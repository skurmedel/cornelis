#include <cornelis/Scene.hpp>

#include "MathHelpers.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating.hpp>

using namespace cornelis;

TEST_CASE("Scene: constructor") {
    Scene scene;

    // Check that there is a default camera.
    CHECK(scene.camera().get() != nullptr);
}

TEST_CASE("Scene: setCamera") {
    Scene scene;

    auto camera = std::make_shared<PerspectiveCamera>();
    scene.setCamera(camera);

    CHECK(scene.camera().get() == camera.get());

    CHECK_THROWS(scene.setCamera(nullptr));
}

TEST_CASE("Scene: object spheres") {
    Scene scene;

    auto &spheres = scene.spheres();
    CHECK(spheres.size() == 0);
}