#include <catch2/catch_test_macros.hpp>

#include <cornelis/FrameBuffer.hpp>

using namespace cornelis;

TEST_CASE("FrameBuffer: constructor") {
    RGBFrameBuffer fb(PixelRect(128, 64));

    CHECK_THROWS(RGBFrameBuffer(PixelRect(128, 0)));
    CHECK_THROWS(RGBFrameBuffer(PixelRect(0, 128)));
    CHECK_THROWS(RGBFrameBuffer(PixelRect(0, 0)));
}

TEST_CASE("FrameBuffer: aspect()") {
    RGBFrameBuffer fb(PixelRect(128, 64));
    CHECK(fb.aspect() == 2.0f); // 128.0/64.0 is exactly representible.
}

TEST_CASE("FrameBuffer: operator()") {
    RGBFrameBuffer fb(PixelRect(128, 64));

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

TEST_CASE("quantizeTo8bit: float") {
    CHECK(quantizeTo8bit(1.0) == uint8_t(255));
    CHECK(quantizeTo8bit(0.0) == uint8_t(0));
    CHECK(quantizeTo8bit(0.5) == uint8_t(128));
    // saturates.
    CHECK(quantizeTo8bit(+5.0) == uint8_t(255));
    CHECK(quantizeTo8bit(-5.0) == uint8_t(0));
}

TEST_CASE("quantizeTo8bit: SRGB") {
    CHECK(quantizeTo8bit(SRGB{{5.0, 1.0, 0.0}}) == std::array<uint8_t, 3>{255, 255, 0});
}