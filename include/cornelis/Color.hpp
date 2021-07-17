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
 * A linear sRGB triplet. Most of the methods treat it as a vector in three dimensional sRGB space.
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

    /**
     * Access the component of a given index.
     */
    auto operator()(std::size_t c) const noexcept -> float const & { return values_[c]; }
    auto operator()(std::size_t c) noexcept -> float & { return values_[c]; }

    inline static constexpr RGB black() { return {}; };
    inline static constexpr RGB red() { return {1, 0, 0}; }
    inline static constexpr RGB green() { return {0, 1, 0}; }
    inline static constexpr RGB blue() { return {0, 0, 1}; }

    /**
     * Vector addition.
     */
    auto operator+(RGB const &) const noexcept -> RGB;
    /**
     * Vector subtraction.
     */
    auto operator-(RGB const &) const noexcept -> RGB;

    /**
     * Vector and scalar multiplication.
     */
    auto operator*(float scalar) const noexcept -> RGB;

    friend auto toSRGB(RGB const &) -> SRGB;

  private:
    float values_[3];
};

/**
 * Gamma corrects a RGB triplet using the sRGB transfer function. This is an invertible operation.
 */
auto toSRGB(RGB const &) -> SRGB;

} // namespace cornelis
