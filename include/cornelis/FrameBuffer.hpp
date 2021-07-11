#pragma once

#include <vector>

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

  private:
    BBoxi dims_;
    container_type values_;
};

template <typename TPixel>
inline FrameBuffer<TPixel>::FrameBuffer(PixelCoord dimensions)
    : dims_{{0, 0}, {dimensions.i - 1, dimensions.j - 1}}, values_{width() * height()} {
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
} // namespace cornelis
