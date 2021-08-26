#pragma once

#include <vector>

#include <cornelis/Math.hpp>
#include <cornelis/PRNG.hpp>

namespace cornelis {
using TileCoord = PixelCoord;

struct TileInfo {
    explicit TileInfo(std::size_t number, PixelRect pBounds)
        : tileNumber(number), bounds(pBounds), randomGen() {}
    /**
     * A unique identifier for this tile.
     */
    std::size_t tileNumber;
    /**
     * The bounds, in pixel space, of this tile. Equivalently, the pixels that belong to this tile.
     */
    PixelRect bounds;
    /**
     * A random generator specific to this tile. Is only supposed to be used by one thread at a
     * time.
     *
     * As long as this is only used for samples for pixels belonging to this tile, we can ensure
     * that the same seed yields the same image; at least on the same machine.
     *
     * At construction, this is just a default seed generator.
     */
    PRNG randomGen;
};

/**
 * Describes a partition of the frame's pixel into tiles. A tile is sometimes called a bucket, and
 * is a rectangular grouping of pixels.
 *
 * Currently, this generates tiles in a left-to-right, bottom-to-top scheme, and the tile numbers
 * reflect this. In the future, other schemes may be supported, like spirals or a Hilbert curve,
 * as such a user shouldn't expect the tile number to be much more than an identifier.
 */
class FrameTiling {
  public:
    using container_type = std::vector<TileInfo>;
    using iterator = container_type::iterator;
    using const_iterator = container_type::const_iterator;
    using value_type = container_type::value_type;
    using reference = container_type::reference;

    explicit FrameTiling(PixelRect dimensions, PixelRect maxTileSize = PixelRect{32, 32});

    auto begin() const noexcept -> const_iterator;
    auto end() const noexcept -> const_iterator;
    auto begin() noexcept -> iterator;
    auto end() noexcept -> iterator;

    /** Number of tiles in total. */
    auto size() const noexcept -> std::size_t;

    auto operator[](std::size_t k) const -> TileInfo const &;

  private:
    container_type tiles_;
};
} // namespace cornelis
