#pragma once

#include <cornelis/Math.hpp>

#include <sstream>

#include <catch2/catch_tostring.hpp>

namespace Catch {
template <>
struct StringMaker<cornelis::V3> {
    static std::string convert(cornelis::V3 const &v) {
        std::stringstream output;
        output << "V3(" << v[0] << ", " << v[1] << ", " << v[2] << ")";
        return output.str();
    }
};
} // namespace Catch