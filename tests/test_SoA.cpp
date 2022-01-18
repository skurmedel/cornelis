#include <catch2/catch_test_macros.hpp>

#include <vector>

#include <cornelis/SoA.hpp>

using namespace cornelis;

TEST_CASE("SoAObject get") {
    SoAObject<PositionTag, DirectionTag> obj;

    auto positions = obj.get<PositionTag>();
    auto directions = obj.get<DirectionTag>();
    CHECK(positions.size() == 0);
    CHECK(directions.size() == 0);

    SoAObject<PositionTag, TransformTag> obj2(121);

    positions = obj2.get<PositionTag>();
    auto transforms = obj2.get<TransformTag>();
    CHECK(positions.size() == 121);
    CHECK(transforms.size() == 121);
}