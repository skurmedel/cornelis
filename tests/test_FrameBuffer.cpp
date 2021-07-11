#include <catch2/catch_test_macros.hpp>

#include <cornelis/FrameBuffer.hpp>

using namespace cornelis;

TEST_CASE("FrameBuffer: constructor") {
    RGBFrameBuffer fb({128, 64});

    CHECK_THROWS(RGBFrameBuffer({128, 0}));
    CHECK_THROWS(RGBFrameBuffer({0, 128}));
    CHECK_THROWS(RGBFrameBuffer({0, 0}));
}

TEST_CASE("FrameBuffer: aspect()") {
    RGBFrameBuffer fb({128, 64});
    CHECK(fb.aspect() == 2.0f); // 128.0/64.0 is exactly representible.
}

TEST_CASE("FrameBuffer: operator()") {
    RGBFrameBuffer fb({128, 64});

    // "zero" by default.
    CHECK(fb(0, 0)(0) == 0.0f);
    CHECK(fb(0, 0)(1) == 0.0f);
    CHECK(fb(0, 0)(2) == 0.0f);

    // Setting a value
    fb(0, 0) = RGB::red();
    CHECK(fb(0, 0)(0) == 1.0f);
    CHECK(fb(0, 0)(1) == 0.0f);
    CHECK(fb(0, 0)(2) == 0.0f);
}