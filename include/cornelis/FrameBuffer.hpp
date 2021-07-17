#pragma once

#include <algorithm>
#include <array>
#include <vector>

#include <type_traits>

#include <cornelis/Color.hpp>
#include <cornelis/Expects.hpp>
#include <cornelis/Math.hpp>

namespace cornelis {
/**
 * Represents a FrameBuffer of some size.
 */
template <typename TPixel>
class FrameBuffer {
  public:
    using value_type = TPixel;
    using container_type = std::vector<value_type>;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

    /**
     * \pre !dimensions.zero()
     */
    explicit FrameBuffer(PixelCoord dimensions);
    auto operator()(PixelCoord::value_type i, PixelCoord::value_type j) const noexcept
        -> value_type const &;
    auto operator()(PixelCoord::value_type i, PixelCoord::value_type j) noexcept -> value_type &;
    auto operator()(PixelCoord const &) const noexcept -> value_type const &;
    auto operator()(PixelCoord const &) noexcept -> value_type &;

    /**
     * Width to height ratio: width / height.
     */
    auto aspect() const noexcept -> double;

    auto width() const noexcept -> PixelCoord::value_type { return dims_.max().i + 1; }
    auto height() const noexcept -> PixelCoord::value_type { return dims_.max().j + 1; }

    auto begin() noexcept -> iterator { return std::begin(values_); }

    auto end() noexcept -> iterator { return std::end(values_); }

    auto begin() const noexcept -> const_iterator { return std::begin(values_); }

    auto end() const noexcept -> const_iterator { return std::end(values_); }

    auto data() const noexcept -> value_type const * { return values_.data(); }

  private:
    BBoxi dims_;
    container_type values_;
};

template <typename TPixel>
inline FrameBuffer<TPixel>::FrameBuffer(PixelCoord dimensions)
    : dims_{{0, 0}, {std::abs(dimensions.i - 1), std::abs(dimensions.j - 1)}},
      values_{static_cast<std::size_t>(width() * height())} {
    CORNELIS_EXPECTS(dimensions.i != 0 && dimensions.j != 0,
                     "We do not support infinitely thin images.");
}

template <typename TPixel>
inline auto FrameBuffer<TPixel>::operator()(PixelCoord::value_type i,
                                            PixelCoord::value_type j) const noexcept
    -> value_type const & {
    return values_[j * width() + i];
}
template <typename TPixel>
inline auto FrameBuffer<TPixel>::operator()(PixelCoord::value_type i,
                                            PixelCoord::value_type j) noexcept -> value_type & {
    return values_[j * width() + i];
}
template <typename TPixel>
inline auto FrameBuffer<TPixel>::operator()(PixelCoord const &c) const noexcept
    -> value_type const & {
    return this->operator()(c.i, c.j);
}
template <typename TPixel>
inline auto FrameBuffer<TPixel>::operator()(PixelCoord const &c) noexcept -> value_type & {
    return this->operator()(c.i, c.j);
}

template <typename TPixel>
inline auto FrameBuffer<TPixel>::aspect() const noexcept -> double {
    return double(width()) / height();
}

using RGBFrameBuffer = FrameBuffer<RGB>;
using SRGBFrameBuffer = FrameBuffer<SRGB>;

inline auto quantizeTo8bit(double v) -> uint8_t {
    v = std::round(255.0 * v);
    return uint8_t(std::clamp(v, 0.0, 255.0));
}

inline auto quantizeTo8bit(SRGB const &v) -> std::array<uint8_t, 3> {
    std::array<uint8_t, 3> values = {
        quantizeTo8bit(v(0)), quantizeTo8bit(v(1)), quantizeTo8bit(v(2))};
    return values;
}

inline auto quantizeTo8bit(SRGBFrameBuffer const &fb) -> FrameBuffer<std::array<uint8_t, 3>> {
    FrameBuffer<std::array<uint8_t, 3>> output({fb.width(), fb.height()});
    std::transform(
        fb.begin(), fb.end(), output.begin(), [](auto const &v) { return quantizeTo8bit(v); });
    return output;
}

} // namespace cornelis
