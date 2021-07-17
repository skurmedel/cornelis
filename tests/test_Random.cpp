#include <algorithm>
#include <array>

#include <tbb/parallel_do.h>

#include <catch2/catch_test_macros.hpp>

#include <cornelis/Random.hpp>

using namespace cornelis;

TEST_CASE("setSeed") {
    random::setSeed(1234);

    std::vector<float> a{{-1.0f}};
    std::vector<float> b{{-1.0f}};

    random::uniform01(a);

    random::setSeed(1234);

    random::uniform01(b);

    // These should be bit for bit equal.
    CHECK(a.front() == b.front());
}

TEST_CASE("uniform01: threading") {
    std::vector<float> indices = {0, 1, 2};
    std::vector<std::vector<float>> vectors;
    vectors.resize(indices.size());

    random::setSeed(1234);

    tbb::parallel_do(std::begin(indices), std::end(indices), [&](std::size_t i) -> void {
        vectors[i].resize(10);
        random::uniform01(vectors[i]);
    });

    // It should be extremely unlikely that any of these values are the same if we are indeed
    // using different points of the period per thread.
    for (int i = 0; i < indices.size(); i++) {
        for (int j = 0; j < indices.size(); j++) {
            if (i == j)
                continue;
            CHECK(vectors[i] != vectors[j]);
        }
    }
}