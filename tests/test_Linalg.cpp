#include <catch2/catch_test_macros.hpp>

#include <vector>

#include <cornelis/Linalg.hpp>

using namespace cornelis;

TEST_CASE("float4: constructors") {
    float4 a{1, 2, 3, 4};
    CHECK(a.values[0] == 1.0f);
    CHECK(a.values[1] == 2.0f);
    CHECK(a.values[2] == 3.0f);
    CHECK(a.values[3] == 4.0f);

    float4 b = float4::point3(3.0f, 6.0f, 9.0f);
    CHECK(b.values[0] == 3.0f);
    CHECK(b.values[1] == 6.0f);
    CHECK(b.values[2] == 9.0f);
    CHECK(b.values[3] == 1.0f);

    float4 c = float4::normal3(3.0f, 6.0f, 9.0f);
    CHECK(c.values[0] == 3.0f);
    CHECK(c.values[1] == 6.0f);
    CHECK(c.values[2] == 9.0f);
    CHECK(c.values[3] == 0.0f);
}
/*

TEST_CASE("matrixMultiply: simple cases") {
    struct Case {
        float4x4 A;
        float4 x;
        float4 expected;
    } cases[] = {
        {
            float4x4::identityMatrix(),
            float4{1.f, 2.f, 3.f, 4.f},
            float4{1.f, 2.f, 3.f, 4.f},
        },
        {
            float4x4::scalingMatrix(float4{1.0f, 2.0f, 3.0f, 4.0f}),
            float4{1.f, 2.f, 3.f, 4.f},
            float4{1.f, 4.f, 9.f, 16.f},
        },
        {
            float4x4::scalingMatrix(float4{0.0f, 2.0f, 0.0f, 4.0f}),
            float4{1.f, 2.f, 3.f, 4.f},
            float4{0.f, 4.f, 0.f, 16.f},
        },
    };
    for (auto const &testCase : cases) {
        float4 y;
        matrixMultiply(testCase.A, testCase.x, y);
        CHECK(testCase.expected == y);
    }
}

TEST_CASE("matrixMultiply: &x == &y") {
    float4x4 A = float4x4::scalingMatrix({2.0f, 3.0f, 4.0f, 5.0f});
    float4 x{{0.f, 2.f, 3.f, 4.f}};
    float4 expected{{0.f, 6.f, 12.f, 20.f}};

    matrixMultiply(A, x, x);
    CHECK(x == expected);
}

TEST_CASE("transformRays") {
    float4x4 A = float4x4::scalingMatrix(float4{2.0f, 3.0f, 4.0f, 1.0f});
    // Add some translation.
    A.values[0 * 4 + 3] = 2.0;
    A.values[1 * 4 + 3] = 2.0;
    A.values[2 * 4 + 3] = 2.0;

    ray4 ray1{float4::point3(-2, 2, 2), float4::normal3(1, 1, 1)};
    std::vector<ray4> rays = {ray1};

    transformRays(A, rays);

    CHECK(rays[0].pos == float4{-4 + 2, 6 + 2, 4 * 2 + 2, 1});
    CHECK(rays[0].dir == float4{2, 3, 4, 0});
}

TEST_CASE("Vec4SoAParam: constructor") {
    std::vector<float> elements = {1.0f, 2.0f, 3.0f};
    std::vector<float> elements2 = {1.0f, 2.0f, 3.0f, 4.0f};

    Vec4SoAParam p1(elements, elements, elements, elements);
    CHECK(p1.x.data() == p1.y.data() == p1.z.data() == p1.w.data());

    // Incompatible lengths
    CHECK_THROWS(Vec4SoAParam(elements, elements, elements, elements2));
}*/