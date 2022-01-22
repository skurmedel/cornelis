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
struct RGB {
    using element_type = float;
    static const std::size_t N = 3;

    constexpr RGB() : values{0} {};
    constexpr RGB(float r, float g, float b) : values{r, g, b} {};

    constexpr RGB(RGB const &) = default;
    constexpr RGB(RGB &&) = default;
    constexpr auto operator=(RGB const &) -> RGB & = default;
    constexpr auto operator=(RGB &&) -> RGB & = default;

    /**
     * Access the component of a given index.
     */
    auto operator()(std::size_t c) const noexcept -> float const & { return values[c]; }
    auto operator()(std::size_t c) noexcept -> float & { return values[c]; }

    inline static constexpr RGB black() { return {}; };
    inline static constexpr RGB red() { return {1, 0, 0}; }
    inline static constexpr RGB green() { return {0, 1, 0}; }
    inline static constexpr RGB blue() { return {0, 0, 1}; }

    auto operator+=(RGB const &) noexcept -> RGB &;

    auto operator/(float scalar) const noexcept -> RGB;

    /**
     * Componentwise multiplication.
     */
    auto operator*=(RGB const &) noexcept -> RGB &;

    friend auto toSRGB(RGB const &) -> SRGB;

    auto operator<=>(RGB const &) const = default;
    float values[3];
};

/**
 * Gamma corrects a RGB triplet using the sRGB transfer function. This is an invertible operation.
 */
auto toSRGB(RGB const &) -> SRGB;

} // namespace cornelis
