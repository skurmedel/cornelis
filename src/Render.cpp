#include <algorithm>
#include <numeric>
#include <vector>

#include <tbb/blocked_range2d.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>

#include "extern/stb_image_write.h"

#include <cornelis/FrameBuffer.hpp>
#include <cornelis/PRNG.hpp>
#include <cornelis/Render.hpp>
#include <cornelis/Tiles.hpp>

namespace cornelis {
struct NormalizedFrameBufferCoord {
    NormalizedFrameBufferCoord(PixelCoord pixel, PixelCoord fbSize)
        : dx(1.0f / fbSize.i), dy(1.0f / fbSize.j), x(pixel.i * dx), y(pixel.j * dy) {}

    NormalizedFrameBufferCoord(NormalizedFrameBufferCoord const &) = default;
    NormalizedFrameBufferCoord(NormalizedFrameBufferCoord &&) = default;

    float dx, dy, x, y;
};

// Generate camera rays for the pixel given in normalized frame buffer coordinates.
auto generateCameraRays(PerspectiveCameraPtr cam, NormalizedFrameBufferCoord const &coord)
    -> std::vector<Ray> {
    return {(*cam)(coord.x + 0.5f * coord.dx, coord.y + 0.5f * coord.dy)};
}

auto traceRays(std::vector<Ray> const &rays) -> std::vector<RGB> {
    std::vector<RGB> results;
    results.resize(rays.size());
    std::transform(rays.begin(), rays.end(), results.begin(), [&](Ray const &ray) {
        float t0, t1;
        if (ray.intersects(V3(0), 0.25f, t0, t1)) {
            return RGB(1.0f, 0.0, 0.0f);
        } else {
            return RGB::black();
        }
    });
    return results;
}

auto saveImage(RGBFrameBuffer const &fb) -> void {
    SRGBFrameBuffer srgbFb(PixelRect(fb.width(), fb.height()));
    std::transform(fb.begin(), fb.end(), srgbFb.begin(), toSRGB);
    auto data8bit = quantizeTo8bit(srgbFb);

    // Todo: This is a bit iffy if for some reason the elements of data8bit would be padded.
    stbi_write_png(
        "cornelisrender2.png", data8bit.width(), data8bit.height(), 3, data8bit.data(), 0);
}

struct RenderSession::State {
    State(Scene const &sc, RenderOptions opts) : scene(sc), options(std::move(opts)) {}

    Scene const &scene;
    RenderOptions options;
};

RenderSession::RenderSession(Scene const &sc, RenderOptions options)
    : me_{std::make_unique<State>(sc, std::move(options))} {}

RenderSession::~RenderSession() {}

auto RenderSession::render() -> void {
    RGBFrameBuffer fb(PixelRect(128, 64));
    PRNG rootRng;

    puts("Starting render.");

    FrameTiling tiling(PixelRect(fb.width(), fb.height()));
    for (auto &tileInfo : tiling) {
        tileInfo.randomGen = cloneForThread(rootRng, tileInfo.tileNumber);
    }

    tbb::parallel_for_each(std::begin(tiling), std::end(tiling), [&](TileInfo &tileInfo) -> void {
        for (auto j = tileInfo.bounds.min().j; j <= tileInfo.bounds.max().j; j++) {
            for (auto i = tileInfo.bounds.min().i; i <= tileInfo.bounds.max().i; i++) {
                NormalizedFrameBufferCoord screenCoord({i, j}, {fb.width(), fb.height()});

                auto cameraRays = generateCameraRays(me_->scene.camera(), screenCoord);
                auto results = traceRays(cameraRays);

                auto color = std::accumulate(results.begin(), results.end(), RGB::black());
                // Box-filter 0.5f radius
                color = color * (1.0f / results.size());

                fb(i, j) = color;
            }
        }
    });

    puts("Saving image.");
    saveImage(fb);
}
} // namespace cornelis