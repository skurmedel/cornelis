#include <cornelis/Math.hpp>

#include "MathHelpers.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating.hpp>

using namespace cornelis;

// We test all these with float3, which is the prototypical implemnentor.
TEST_CASE("product_ring: float3: operator+") {
    float3 a{1.0, 2.0, 3.0};
    CHECK(a + a == float3{2.0, 4.0, 6.0});
    CHECK(a + (-a) == float3{0});
}

TEST_CASE("product_ring: float3: operator-") {
    float3 a{1.0, 2.0, 3.0};
    CHECK(a - a == float3{0});
    CHECK(a - (-a) == float3{2.0, 4.0, 6.0});
}

TEST_CASE("product_ring: float3: operator*") {
    float3 a{1.0, 2.0, 3.0};
    CHECK(a * a == float3{1.0, 4.0, 9.0});

    CHECK(a * 2.0f == float3{2.0, 4.0, 6.0});
}

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

TEST_CASE("float4: operator==") {
    float4 a{{1.0, 2.0, 3.0, 4.0}};
    CHECK(a == a);
    float4 b{{1.0, 2.0, 3.0, 5.0}};
    CHECK(!(a == b));
    CHECK(a != b);
}

TEST_CASE("float4: operator+") {
    auto a = float4::init(1.0, 2.0, 3.0, 4.0);

    float4 c = a + a;
    CHECK(c == float4::init(2.0, 4.0, 6.0, 8.0));
}

TEST_CASE("float4: operator-") {
    auto a = float4::init(1.0, 2.0, 3.0, 4.0);

    float4 c = a - a;
    CHECK(c == float4::init(0));
    c = c - a;
    CHECK(c == float4::init(-1.0, -2.0, -3.0, -4.0));
}

TEST_CASE("float4: operator*") {
    auto a = float4::init(1.0, 2.0, 3.0, 4.0);
    float4 c = a * float4::init(0.5, -0.5, 2.0, 2.0);

    CHECK(c == float4::init(0.5, -1.0, 6.0, 8.0));
}

TEST_CASE("dot float3") {
    float3 a{1.0, 2.0, 3.0};
    float3 b{-1.0, 2.0, -2.0};

    CHECK(dot(a, b) == -1.0 + 4.0 - 6.0);
}

TEST_CASE("mag2 float3") {
    float3 a{1.0, 2.0, 3.0};
    CHECK(mag2(a) == 1.0 + 4.0 + 9.0);
}

TEST_CASE("rayT") {
    float3 origin{-1.0, 0.0, 1.0};
    float3 dir{1.0, 0.0, 1.0};

    float3 res = rayT(origin, dir, 1.0);
    CHECK(res == float3{0.0, 0.0, 2.0});
}

TEST_CASE("normalize") {
    float3 a{2.0, 2.0, 1.0};
    CHECK_THAT(mag2(normalize(a)), Catch::Matchers::WithinAbs(1.0f, 0.001f));
}

TEST_CASE("cross") {
    float3 a{1.0, 0.0, 0.0};
    float3 b{0.0, 1.0, 0.0};
    CHECK(cross(a, b) == float3{0, 0, 1.0f});
    CHECK(cross(b, a) == float3{0, 0, -1.0f});

    a = float3{0.0, 1.0, 0.0};
    b = float3{0.0, 0.0, 1.0};
    CHECK(cross(a, b) == float3{1.0f, 0, 0});
    CHECK(cross(b, a) == float3{-1.0f, 0, 0});

    a = float3{1.0, 1.0, 0.0};
    b = float3{0.0, 1.0, 1.0};
    CHECK(cross(a, b) == float3{1.0f, -1.0f, 1.0f});
}