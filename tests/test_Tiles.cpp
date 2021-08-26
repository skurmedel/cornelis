#include <cornelis/Tiles.hpp>

#include <algorithm>

#include "MathHelpers.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating.hpp>

using namespace cornelis;

TEST_CASE("FrameTiling: zero axis") {
    CHECK_THROWS(FrameTiling{PixelRect{0, 1}, PixelRect{16, 16}});
    CHECK_THROWS(FrameTiling{PixelRect{1, 0}, PixelRect{16, 16}});
}

TEST_CASE("FrameTiling: bad max tile size") {
    CHECK_THROWS(FrameTiling{PixelRect{5, 5}, PixelRect{0, 16}});
    CHECK_THROWS(FrameTiling{PixelRect{5, 5}, PixelRect{16, 0}});
}

TEST_CASE("FrameTiling: dimensions multiple of tile size") {
    FrameTiling tiling{PixelRect{32, 9}, PixelRect{16, 3}};
    REQUIRE(tiling.size() == 2 * 3);

    // Check that their identifier matches what we find them by.
    for (std::size_t i = 0; i != tiling.size(); i++) {
        CHECK(tiling[i].tileNumber == i);
    }

    for (int i = 0; i != tiling.size(); i++) {
        int x = i % 2;
        int y = i / 2;

        CHECK(tiling[i].bounds ==
              PixelRect(PixelCoord{x * 16, y * 3}, PixelCoord{(x + 1) * 16 - 1, (y + 1) * 3 - 1}));
    }
}