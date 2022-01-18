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

#include <cornelis/SoA.hpp>

namespace cornelis {
struct NormalizedFrameBufferCoord {
    NormalizedFrameBufferCoord(PixelCoord pixel, PixelCoord fbSize)
        : dx(1.0f / fbSize.i), dy(1.0f / fbSize.j), x(pixel.i * dx), y(pixel.j * dy) {}

    NormalizedFrameBufferCoord(NormalizedFrameBufferCoord const &) = default;
    NormalizedFrameBufferCoord(NormalizedFrameBufferCoord &&) = default;

    float dx, dy, x, y;
};

constexpr float RussianRouletteFactor = 0.75;

struct Basis {
    V3 N;
    V3 T;
    V3 B;
};

auto constructBasis(V3 const &N) -> Basis {
    // Invent a tangent.
    V3 helper(0, 1, 0);
    if (abs(N[1]) > 0.95)
        helper = V3(0, 0, 1);
    Basis base{.N = N};
    base.T = (helper.cross(N)).normalize();
    base.B = base.T.cross(N);
    return base;
}

auto randomHemisphere(PRNG &prng) -> V3 {
    float x1 = prng();
    float x2 = prng();

    float a = 2.0 * Pi * x2;
    float b = sqrt(1.0f - x1 * x1);

    return V3(cos(a) * b, sin(a) * b, x1);
}

auto randomHemisphere(PRNG &prng, Basis const &base) -> V3 {
    V3 v = randomHemisphere(prng);
    return v[0] * base.B + v[1] * base.T + v[2] * base.N;
}

constexpr auto randomHemispherePDF() -> float { return 1.0f / (2.0f * Pi); }

struct RayTag {
    using element_type = Ray;
};

struct PathThroughputTag {
    using element_type = RGB;
};

struct LightInTag {
    using element_type = RGB;
};

struct SurfaceHitTag {
    using element_type = SurfaceHitInfo;
};

struct RayBatch : public SoAObject<RayTag, PathThroughputTag, LightInTag, SurfaceHitTag> {
    RayBatch(std::size_t n) : SoAObject(n), activeList(n) {
        std::iota(std::begin(activeList), std::end(activeList), 0);
        auto throughput = get<PathThroughputTag>();
        std::fill(std::begin(throughput), std::end(throughput), RGB(1.0f, 1.0f, 1.0f));
    }

    auto throughput(std::size_t k) -> RGB { return get<PathThroughputTag>()[k]; }

    auto scaleThroughput(std::size_t k, RGB const &p) -> void { get<PathThroughputTag>()[k] *= p; }

    auto accumulateLight(std::size_t k, RGB light) -> void {
        get<LightInTag>()[k] += throughput(k) * light;
    }

    auto ray(std::size_t k) -> Ray & { return get<RayTag>()[k]; }

    auto hit(std::size_t k) -> SurfaceHitInfo & { return get<SurfaceHitTag>()[k]; }

    std::vector<std::size_t> activeList;
};

constexpr int SamplesAA = 16000;

// Generate camera rays for the pixel given in normalized frame buffer coordinates.
auto generateCameraRays(TileInfo &tileInfo,
                        PerspectiveCameraPtr cam,
                        NormalizedFrameBufferCoord const &coord,
                        RayBatch &raybatch) -> void {
    // Completely random sampling is known to be substandard, we should use a low-discrepancy
    // sequence of points, like multi-jittered sampling or Sobol sequences. We will address this in
    // Milestone 3 when we have generators for these type of sequences.
    for (auto &ray : raybatch.get<RayTag>()) {
        float phi1 = tileInfo.randomGen();
        float phi2 = tileInfo.randomGen();
        ray = (*cam)(coord.x + phi1 * coord.dx, coord.y + phi2 * coord.dy);
    }
}

auto randomSphere(PRNG &prng) -> V3 {
    // TODO: we can use identities to tidy this up.
    auto theta = 2.0f * cornelis::Pi * prng();
    auto phi = std::acos(2.0f * prng() - 1.0f);

    return V3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
}

auto intersect(Scene const &scene, RayBatch &raybatch) -> void {
    std::vector<std::size_t> stillActive;
    for (auto k : raybatch.activeList) {
        SurfaceHitInfo hit{.t0 = INFINITY};
        bool anyHit = false;
        for (std::size_t i = 0; i < scene.spheres().size(); ++i) {
            SurfaceHitInfo candidateHit;
            if (scene.spheres().geometry(i).intersects(raybatch.ray(k), candidateHit)) {
                anyHit = true;
                if (candidateHit.t0 < hit.t0) {
                    hit = candidateHit;
                    hit.mat = &scene.spheres().material(i);
                }
            }
        }
        if (anyHit) {
            stillActive.push_back(k);
            raybatch.hit(k) = hit;
        }
    }
    raybatch.activeList = stillActive;
}

auto accumulateAndBounce(RayBatch &raybatch, PRNG &randomGen) -> void {
    std::vector<std::size_t> stillActive;
    for (auto k : raybatch.activeList) {
        V3 const w_out = -raybatch.ray(k).dir();

        auto const hit = raybatch.hit(k);

        // TODO: We can chose a much better russian roulette factor.
        auto const prob = RussianRouletteFactor;
        auto const P = hit.P;
        auto const N = hit.N;
        auto const L_e = hit.mat->emission(P);

        raybatch.accumulateLight(k, L_e);

        if (prob <= randomGen()) {
            // We killed the ray tree due to russian roulette.
            continue;
        }

        BSDF const &bsdf = hit.mat->bsdf(P, N);
        // TODO: we can do much better here by importance sampling.
        V3 w_in = randomHemisphere(randomGen, constructBasis(N));
        // float pdf = bsdf.pdf(w_in);
        float pdf = randomHemispherePDF();

        // TODO: we should probably chose prob here based on the material at least.
        // Create new ray for this bounce.
        raybatch.ray(k) = Ray(P + w_in * 0.00001f, w_in);
        // Set light term scale to be accumulated.
        raybatch.scaleThroughput(
            k, bsdf(w_in, w_out) * abs(std::max(w_in.dot(N), 0.0f)) / (pdf * prob));

        stillActive.push_back(k);
    }
    raybatch.activeList = stillActive;
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
                // printf("Rendering tile %zu on thread %d: pixel %u %u\n",
                // tileInfo.tileNumber,
                // tbb::this_task_arena::current_thread_index(),
                // i, j);
                NormalizedFrameBufferCoord screenCoord({i, j}, {fb.width(), fb.height()});

                RayBatch raybatch(SamplesAA);
                generateCameraRays(tileInfo, me_->scene.camera(), screenCoord, raybatch);

                while (raybatch.activeList.size() > 0) {
                    intersect(me_->scene, raybatch);
                    accumulateAndBounce(raybatch, tileInfo.randomGen);
                }

                RGB color = RGB::black();
                for (auto const &term : raybatch.get<LightInTag>()) {
                    color += term;
                }
                // Box-filter 0.5f radius
                color = color * (1.0f / SamplesAA);

                fb(i, j) = color;
            }
        }
    });

    puts("Saving image.");
    saveImage(fb);
}
} // namespace cornelis