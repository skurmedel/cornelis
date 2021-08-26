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

constexpr int SamplesAA = 16;

// Generate camera rays for the pixel given in normalized frame buffer coordinates.
auto generateCameraRays(TileInfo &tileInfo,
                        PerspectiveCameraPtr cam,
                        NormalizedFrameBufferCoord const &coord) -> std::vector<Ray> {
    std::vector<Ray> rays;
    rays.reserve(SamplesAA);

    // Completely random sampling is known to be substandard, we should use a low-discrepancy
    // sequence of points, like multi-jittered sampling or Sobol sequences. We will address this in
    // Milestone 3 when we have generators for these type of sequences.
    for (auto k = 0; k < SamplesAA; k++) {
        float phi1 = tileInfo.randomGen.next();
        float phi2 = tileInfo.randomGen.next();
        rays.emplace_back((*cam)(coord.x + phi1 * coord.dx, coord.y + phi2 * coord.dy));
    }

    return rays;
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
    RGBFrameBuffer fb(PixelRect(512, 256));
    PRNG rootRng;

    puts("Starting render.");

    FrameTiling tiling(PixelRect(fb.width(), fb.height()));
    // Set up PRNGs to start at different points in the period.
    for (auto &tileInfo : tiling) {
        tileInfo.randomGen = cloneForThread(rootRng, tileInfo.tileNumber);
    }

    tbb::parallel_for_each(std::begin(tiling), std::end(tiling), [&](TileInfo &tileInfo) -> void {
        printf("Rendering tile %d on thread %d.\n",
               tileInfo.tileNumber,
               tbb::this_task_arena::current_thread_index());

        for (auto j = tileInfo.bounds.min().j; j <= tileInfo.bounds.max().j; j++) {
            for (auto i = tileInfo.bounds.min().i; i <= tileInfo.bounds.max().i; i++) {
                NormalizedFrameBufferCoord screenCoord({i, j}, {fb.width(), fb.height()});

                auto cameraRays = generateCameraRays(tileInfo, me_->scene.camera(), screenCoord);
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