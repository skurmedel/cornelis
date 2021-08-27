#include <cornelis/Math.hpp>

#include "MathHelpers.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating.hpp>

using namespace cornelis;

TEST_CASE("PixelRect: default constructor") {
    PixelRect rect;
    CHECK(rect.width() == 1);
    CHECK(rect.height() == 1);
    CHECK(rect.area() == 1);
}

TEST_CASE("PixelRect: two point constructor") {
    // Deliberately passed in the points in non-canonical order.
    PixelRect rect(PixelCoord{10, 2}, PixelCoord{-1, 1});
    CHECK(rect.width() == 12);
    CHECK(rect.height() == 2);
    CHECK(rect.area() == 12 * 2);
}

TEST_CASE("P2: empty constructor") {
    P2 v;
    auto [x, y] = v;
    CHECK(x == 0);
    CHECK(y == 0);
}

TEST_CASE("P2: two variable constructor") {
    P2 v(-1.0, 2.0);
    auto [x, y] = v;
    CHECK(x == -1.0);
    CHECK(y == 2.0);
}