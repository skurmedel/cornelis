#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating.hpp>

#include <cornelis/Color.hpp>

using namespace cornelis;

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
