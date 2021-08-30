#include <algorithm>
#include <numeric>
#include <vector>

#include <tbb/blocked_range2d.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>

#include "extern/stb_image_write.h"

#include <cornelis/Color.hpp>
#include <cornelis/FrameBuffer.hpp>
#include <cornelis/Materials.hpp>
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

constexpr int SamplesAA = 2048;

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
        float phi1 = tileInfo.randomGen();
        float phi2 = tileInfo.randomGen();
        rays.emplace_back((*cam)(coord.x + phi1 * coord.dx, coord.y + phi2 * coord.dy));
    }

    return rays;
}
constexpr float RussianRouletteFactor = 0.75;

auto randomSphere(PRNG &prng) -> V3 {
    auto theta = 2.0f * cornelis::Pi * prng();
    auto phi = std::acos(2.0f * prng() - 1.0f);

    return V3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
}

auto lightIn(Scene const &scene, TileInfo &tileInfo, Ray const &ray) -> RGB {
    V3 const w_out = -ray.dir();

    SurfaceHitInfo hit;
    hit.t0 = INFINITY;
    bool anyHit = false;
    StandardMaterial const *mat = nullptr;
    for (std::size_t i = 0; i < scene.spheres().size(); ++i) {
        SurfaceHitInfo candidateHit;
        if (scene.spheres().geometry(i).intersects(ray, candidateHit)) {
            anyHit = true;
            if (candidateHit.t0 < hit.t0) {
                hit = candidateHit;
                mat = &scene.spheres().material(i);
            }
        }
    }
    if (!anyHit)
        return RGB::black();

    // TODO: We can chose a much better russian roulette factor.
    auto const prob = RussianRouletteFactor;
    auto const P = hit.P;
    auto const N = hit.N;
    auto const L_e = mat->emission(P);
    if (prob <= tileInfo.randomGen())
        return L_e;
    BSDF const &bsdf = mat->bsdf(P, N);

    // TODO: we can do much better here by importance sampling.
    V3 w_in = randomSphere(tileInfo.randomGen);
    // TODO: we should probably chose prob here based on the material at least.

    RGB L_i = lightIn(scene, tileInfo, Ray(P, w_in));
    return L_e + L_i * bsdf(w_in, w_out) * abs(std::max(w_in.dot(N), 0.0f)) / prob;
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

    FrameTiling tiling(PixelRect(fb.width(), fb.height()), PixelRect{16, 16});
    // Set up PRNGs to start at different points in the period.
    for (auto &tileInfo : tiling) {
        tileInfo.randomGen = cloneForThread(rootRng, tileInfo.tileNumber);
    }

    tbb::parallel_for_each(std::begin(tiling), std::end(tiling), [&](TileInfo &tileInfo) -> void {
        printf("Rendering tile %zu on thread %d.\n",
               tileInfo.tileNumber,
               tbb::this_task_arena::current_thread_index());

        for (auto j = tileInfo.bounds.min().j; j <= tileInfo.bounds.max().j; j++) {
            for (auto i = tileInfo.bounds.min().i; i <= tileInfo.bounds.max().i; i++) {
                NormalizedFrameBufferCoord screenCoord({i, j}, {fb.width(), fb.height()});

                auto cameraRays = generateCameraRays(tileInfo, me_->scene.camera(), screenCoord);
                std::vector<RGB> results;
                results.resize(cameraRays.size());

                std::transform(cameraRays.begin(),
                               cameraRays.end(),
                               results.begin(),
                               [&](Ray const &ray) { return lightIn(me_->scene, tileInfo, ray); });

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