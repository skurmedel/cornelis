#include <cornelis/Expects.hpp>
#include <cornelis/Tiles.hpp>

namespace cornelis {
FrameTiling::FrameTiling(PixelRect dimensions, PixelRect maxTileSize) : tiles_{} {
    PixelRect::element_type numX = dimensions.width() / maxTileSize.width();
    PixelRect::element_type numY = dimensions.height() / maxTileSize.height();
    auto spillX = dimensions.width() % maxTileSize.width();
    auto spillY = dimensions.height() % maxTileSize.height();
    if (spillX != 0)
        numX += 1;
    if (spillY != 0)
        numY += 1;

    std::size_t number = 0;
    tiles_.reserve(numX * numY);
    for (decltype(numY) j = 0; j < numY; j++) {
        for (decltype(numX) i = 0; i < numX; i++) {
            PixelCoord pMin{i * maxTileSize.width(), j * maxTileSize.height()};
            PixelCoord pMax{(i + 1) * maxTileSize.width() - 1, (j + 1) * maxTileSize.height() - 1};
            if (i == numX - 1 && spillX != 0)
                pMax.i = spillX - 1;
            if (j == numY - 1 && spillY != 0)
                pMax.j = spillY - 1;

            tiles_.emplace_back(number++, PixelRect{pMin, pMax});
        }
    }
}

auto FrameTiling::begin() const noexcept -> const_iterator { return std::begin(tiles_); }
auto FrameTiling::end() const noexcept -> const_iterator { return std::end(tiles_); }
auto FrameTiling::begin() noexcept -> iterator { return std::begin(tiles_); }
auto FrameTiling::end() noexcept -> iterator { return std::end(tiles_); }

auto FrameTiling::size() const noexcept -> std::size_t { return std::size(tiles_); }

auto FrameTiling::operator[](std::size_t k) const -> TileInfo const & {
    // At the moment they are bijective.
    return tiles_[k];
}
} // namespace cornelis