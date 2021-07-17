#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating.hpp>

#include <cornelis/Color.hpp>

using namespace cornelis;

TEST_CASE("RGB: operator+") {
    RGB rgb(1.0f, -2.0f, 3.0f);
    RGB result = rgb + rgb;

    CHECK(result(0) == 2.0f);
    CHECK(result(1) == -4.0f);
    CHECK(result(2) == 6.0f);

    result = rgb + RGB(-1.0f, 2.0f, -3.0f);

    CHECK(result(0) == 0.0f);
    CHECK(result(1) == 0.0f);
    CHECK(result(2) == 0.0f);
}

TEST_CASE("RGB: operator-") {
    RGB rgb(1.0f, -2.0f, 3.0f);
    RGB result = rgb - rgb;

    CHECK(result(0) == 0.0f);
    CHECK(result(1) == 0.0f);
    CHECK(result(2) == 0.0f);

    result = rgb - RGB(-1.0f, 2.0f, -3.0f);

    CHECK(result(0) == 2.0f);
    CHECK(result(1) == -4.0f);
    CHECK(result(2) == 6.0f);
}

TEST_CASE("RGB: operator* scalar") {
    RGB rgb(1.0f, -2.0f, 4.0f);
    RGB result = rgb * 0.5f;

    CHECK(result(0) == 0.5f);
    CHECK(result(1) == -1.0f);
    CHECK(result(2) == 2.0f);
}

TEST_CASE("toSRGB(RGB)") {
    RGB c = RGB::black();
    SRGB s = toSRGB(c);
    CHECK(s(0) == 0.0);
    CHECK(s(1) == 0.0);
    CHECK(s(2) == 0.0);

    c = RGB{0.5, 0.5, 0.5};
    s = toSRGB(c);
    CHECK_THAT(s(0), Catch::Matchers::WithinAbs(0.7353, 0.01));
    CHECK_THAT(s(1), Catch::Matchers::WithinAbs(0.7353, 0.01));
    CHECK_THAT(s(2), Catch::Matchers::WithinAbs(0.7353, 0.01));

    c = RGB{1.0, 1.0, 1.0};
    s = toSRGB(c);
    CHECK_THAT(s(0), Catch::Matchers::WithinAbs(1.0, 0.01));
    CHECK_THAT(s(1), Catch::Matchers::WithinAbs(1.0, 0.01));
    CHECK_THAT(s(2), Catch::Matchers::WithinAbs(1.0, 0.01));
}
