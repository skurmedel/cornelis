#pragma once

#include <cornelis/Math.hpp>

namespace cornelis {
/**
 * A non-linear sRGB color.
 */
struct SRGB {
    auto operator()(std::size_t c) const noexcept -> float const & { return values[c]; }
    auto operator()(std::size_t c) noexcept -> float & { return values[c]; }

    float values[3];
};

/**
 * A linear sRGB triplet.
 */
class RGB {
  public:
    using value_type = float;

    constexpr RGB() : values_{0} {};
    constexpr RGB(float r, float g, float b) : values_{r, g, b} {};
    
    constexpr RGB(RGB const &) = default;
    constexpr RGB(RGB &&) = default;
    constexpr auto operator=(RGB const &) -> RGB & = default;
    constexpr auto operator=(RGB &&) -> RGB & = default;

    auto operator()(std::size_t c) const noexcept -> float const & { return values_[c]; }
    auto operator()(std::size_t c) noexcept -> float & { return values_[c]; }

    inline static constexpr RGB black() { return {}; };
    inline static constexpr RGB red() { return {1, 0, 0}; }
    inline static constexpr RGB green() { return {0, 1, 0}; }
    inline static constexpr RGB blue() { return {0, 0, 1}; }

    friend auto toSRGB(RGB const &) -> SRGB;

  private:
    float values_[3];
};

auto toSRGB(RGB const &) -> SRGB;

} // namespace cornelis
