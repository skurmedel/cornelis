#include <algorithm>
#include <atomic>
#include <fmt/core.h>
#include <numeric>
#include <vector>

#include <tbb/blocked_range2d.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>
#include <tbb/task_group.h>

#include <loguru.hpp>

#include "extern/stb_image_write.h"

#include <cornelis/Color.hpp>
#include <cornelis/FrameBuffer.hpp>
#include <cornelis/Materials.hpp>
#include <cornelis/PRNG.hpp>
#include <cornelis/Render.hpp>
#include <cornelis/Scene.hpp>
#include <cornelis/Tiles.hpp>

#include <cornelis/SoA.hpp>

#include <cornelis/Geometry.hpp>

namespace cornelis {
struct NormalizedFrameBufferCoord {
    NormalizedFrameBufferCoord(PixelCoord pixel, PixelCoord fbSize)
        : dx(1.0f / fbSize.i), dy(1.0f / fbSize.j), x(pixel.i * dx), y(pixel.j * dy) {}

    NormalizedFrameBufferCoord(NormalizedFrameBufferCoord const &) = default;
    NormalizedFrameBufferCoord(NormalizedFrameBufferCoord &&) = default;

    float dx, dy, x, y;
};

constexpr float RussianRouletteFactor = 0.75;

auto randomHemisphere(PRNG &prng) -> float3 {
    float x1 = prng();
    float x2 = prng();

    float a = 2.0 * Pi * x2;
    float b = sqrt(1.0f - x1 * x1);

    return float3(cos(a) * b, sin(a) * b, x1);
}

auto randomHemisphere(PRNG &prng, Basis const &base) -> float3 {
    float3 v = randomHemisphere(prng);
    return base.B * v(0) + base.T * v(1) + base.N * v(2);
}

constexpr auto randomHemispherePDF() -> float { return 1.0f / (2.0f * Pi); }

struct PathThroughputTag {
    using element_type = RGB;
};

struct LightInTag {
    using element_type = RGB;
};

struct RayBatch : public SoAObject<tags::PositionX,
                                   tags::PositionY,
                                   tags::PositionZ,
                                   tags::DirectionX,
                                   tags::DirectionY,
                                   tags::DirectionZ,
                                   PathThroughputTag,
                                   LightInTag> {
    RayBatch(std::size_t n) : SoAObject(n), activeList(n) {
        std::iota(std::begin(activeList), std::end(activeList), 0);
        auto throughput = get<PathThroughputTag>();
        std::fill(std::begin(throughput), std::end(throughput), RGB(1.0f, 1.0f, 1.0f));
        auto L_i = get<LightInTag>();
        std::fill(std::begin(L_i), std::end(L_i), RGB::black());
    }

    auto throughput(std::size_t k) -> RGB { return get<PathThroughputTag>()[k]; }

    auto scaleThroughput(std::size_t k, RGB const &p) -> void { get<PathThroughputTag>()[k] *= p; }

    auto accumulateLight(std::size_t k, RGB light) -> void {
        get<LightInTag>()[k] += throughput(k) * light;
    }

    auto rayOrigin(std::size_t k) -> float3 {
        auto [x, y, z] = getPositions(*this);
        return {x[k], y[k], z[k]};
    }

    auto rayDir(std::size_t k) -> float3 {
        auto [x, y, z] = getDirectionSpans(*this);
        return {x[k], y[k], z[k]};
    }

    std::vector<std::size_t> activeList;
};

// Generate camera rays for the pixel given in normalized frame buffer coordinates.
auto generateCameraRays(TileInfo &tileInfo,
                        PerspectiveCamera const &cam,
                        NormalizedFrameBufferCoord const &coord,
                        RayBatch &raybatch) -> void {
    // Completely random sampling is known to be substandard, we should use a low-discrepancy
    // sequence of points, like multi-jittered sampling or Sobol sequences. We will address this in
    // Milestone 3 when we have generators for these type of sequences.
    auto x = raybatch.get<tags::PositionX>();
    for (std::size_t k = 0; k != x.size(); k++) {
        float phi1 = tileInfo.randomGen();
        float phi2 = tileInfo.randomGen();
        auto ray = cam(coord.x + phi1 * coord.dx, coord.y + phi2 * coord.dy);
        setPosition(raybatch, k, float3{ray.eye()[0], ray.eye()[1], ray.eye()[2]});
        setDirection(raybatch, k, float3{ray.dir()[0], ray.dir()[1], ray.dir()[2]});
    }
}

auto randomSphere(PRNG &prng) -> float3 {
    // TODO: we can use identities to tidy this up.
    auto theta = 2.0f * cornelis::Pi * prng();
    auto phi = std::acos(2.0f * prng() - 1.0f);

    return float3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
}

auto intersect(SceneData &scene, RayBatch &raybatch, IntersectionData &intersections) -> void {
    auto [Sx, Sy, Sz] = getPositions(scene.spheres);
    auto radius = scene.spheres.get<tags::Radius>();
    auto materialIds = scene.spheres.get<tags::MaterialId>();

    for (decltype(Sx.size()) i = 0; i != Sx.size(); i++) {
        intersectSphere(getPositions(raybatch),
                        getDirectionSpans(raybatch),
                        float3(Sx[i], Sy[i], Sz[i]),
                        radius[i],
                        materialIds[i],
                        intersections,
                        raybatch.activeList);
    }

    materialIds = scene.planes.get<tags::MaterialId>();
    auto width = scene.planes.get<tags::WidthF>();
    auto height = scene.planes.get<tags::HeightF>();
    auto [Px, Py, Pz] = getPositions(scene.planes);
    auto [PNx, PNy, PNz] = getNormalSpans(scene.planes);
    for (decltype(Px.size()) i = 0; i != Px.size(); i++) {
        intersectPlane(getPositions(raybatch),
                       getDirectionSpans(raybatch),
                       float3(PNx[i], PNy[i], PNz[i]),
                       float3(Px[i], Py[i], Pz[i]),
                       width[i],
                       height[i],
                       materialIds[i],
                       intersections,
                       raybatch.activeList);
    }

    auto params = intersections.get<tags::RayParam0>();
    // Fix up activeList.
    decltype(RayBatch::activeList) newActiveList;
    for (auto k : raybatch.activeList) {
        if (params[k] < INFINITY)
            newActiveList.push_back(k);
    }
    raybatch.activeList = newActiveList;
}

auto accumulateAndBounce(SceneData &scene,
                         RayBatch &raybatch,
                         IntersectionData &intersections,
                         PRNG &randomGen) -> void {
    std::vector<std::size_t> stillActive;
    for (auto k : raybatch.activeList) {
        float3 const w_out = -raybatch.rayDir(k);

        auto [Px, Py, Pz] = getPositions(intersections);
        auto [Nx, Ny, Nz] = getNormalSpans(intersections);
        auto materialIds = intersections.get<tags::MaterialId>();

        auto const &mat = scene.materials[materialIds[k]];
        // TODO: We can chose a much better russian roulette factor.
        auto const prob = RussianRouletteFactor;
        auto const P = float3{Px[k], Py[k], Pz[k]};
        auto const N = float3{Nx[k], Ny[k], Nz[k]};
        auto const L_e = mat.emission(P);

        raybatch.accumulateLight(k, L_e);

        if (prob <= randomGen()) {
            // We killed the ray tree due to russian roulette.
            continue;
        }

        Basis basis = constructBasis(N);
        BRDF const &brdf = mat.brdf(P, N);
        // TODO: we can do much better here by importance sampling.
        float3 w_in = randomHemisphere(randomGen, basis);
        // float3 w_in = normalize(N + randomSphere(randomGen));
        // float pdf = brdf.pdf(w_in);
        float pdf = randomHemispherePDF();

        // TODO: we should probably chose prob here based on the material at least.
        // Create new ray for this bounce.
        // raybatch.ray(k) = Ray(P + w_in * 0.00001f, w_in);
        setPosition(raybatch, k, P + w_in * 0.0001f);
        setDirection(raybatch, k, w_in);
        // Set light term scale to be accumulated.
        raybatch.scaleThroughput(k,
                                 //   (RGB{P[0], P[1], P[2]} * 0.5f + RGB{0.5, 0.5, 0.5}) / Pi *
                                 //       abs(dot(w_in, N)) / (pdf * prob));
                                 brdf(w_in, w_out, N) * abs(dot(w_in, N)) / (pdf * prob));

        stillActive.push_back(k);
    }
    raybatch.activeList = stillActive;
}

auto integrateTile(TileInfo &tileInfo,
                   RenderOptions const &options,
                   SceneData &scene,
                   RGBFrameBuffer &fb) -> void {
    for (auto j = tileInfo.bounds.min().j; j <= tileInfo.bounds.max().j; j++) {
        for (auto i = tileInfo.bounds.min().i; i <= tileInfo.bounds.max().i; i++) {
            // printf("Rendering tile %zu on thread %d: pixel %u %u\n",
            // tileInfo.tileNumber,
            // tbb::this_task_arena::current_thread_index(),
            // i, j);
            NormalizedFrameBufferCoord screenCoord({i, j}, {fb.width(), fb.height()});

            RayBatch raybatch(options.samplesAA);
            generateCameraRays(tileInfo, scene.camera, screenCoord, raybatch);
            IntersectionData intersections(options.samplesAA);

            while (raybatch.activeList.size() > 0) {
                intersect(scene, raybatch, intersections);
                // if (raybatch.activeList.size() > 0)
                //    printf("actives %zu\n", raybatch.activeList.size());
                accumulateAndBounce(scene, raybatch, intersections, tileInfo.randomGen);
                intersections.reset();
            }

            RGB color = RGB::black();
            for (auto const &term : raybatch.get<LightInTag>()) {
                color += term;
            }
            // Box-filter 0.5f radius
            color = color * (1.0f / options.samplesAA);

            fb(i, j) = color;
        }
    }
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
    State(SceneDescription const &sc, RenderOptions opts)
        : sceneDescr(sc), scene(sceneDescr), options(std::move(opts)) {}

    SceneDescription sceneDescr;
    SceneData scene;
    RenderOptions options;
    // Used for book-keeping by the render loop. Most of the values are for reporting or user
    // feedback. Values in this will be changed by multiple threads until the render loop is
    // completed.
    struct Progress {
        // How many rays we expect to trace. For a future progressive mode, this is probably not
        // computable, and this value would be meaningless.
        std::atomic_int64_t primaryRaysTarget;
        // How many primary rays we have launched so far.
        std::atomic_int64_t primayRaysTraced;
        // When rendering in tiled mode, this is the number of tiles that needs to be completed.
        std::atomic_int64_t tilesTarget;
        // When rendering in tiled mode, this is the number of tiles completed.
        std::atomic_int64_t tilesCompleted;

    } progress;
};

RenderSession::RenderSession(SceneDescription const &sc, RenderOptions options)
    : me_{std::make_unique<State>(sc, std::move(options))} {}

RenderSession::~RenderSession() {}

auto RenderSession::render() -> void {
    render([](auto const &progress, auto const &status) -> RenderCommand {
        return RenderCommand::Continue;
    });
}

auto RenderSession::render(ProgressCallback onProgress) -> void {
    // loguru::init(argc, argv);
    // auto logger = spdlog::stdout_logger_mt("console");
    // logger->set_pattern("[%H:%M:%S %z] (t %t): %v");

    RGBFrameBuffer fb(PixelRect(512, 512));
    PRNG rootRng;

    if (me_->options.samplesAA <= 0) { // TODO: create some validation routine.
        printf("AA Samples must be > 0 (not %d).\n", me_->options.samplesAA);
        return;
    }

    LOG_F(INFO, "Starting render session.");
    {
        LOG_SCOPE_F(INFO, "Render Options");
        LOG_F(INFO, "AA Samples {:4}", me_->options.samplesAA);
    }
    {
        LOG_SCOPE_F(INFO, "Scene information");
        LOG_F(INFO, "Spheres   {:4}", me_->scene.spheres.get<tags::PositionX>().size());
        LOG_F(INFO, "Planes    {:4}", me_->scene.planes.get<tags::PositionX>().size());
        LOG_F(INFO, "Materials {:4}", me_->scene.materials.size());
    }

    FrameTiling tiling(PixelRect(fb.width(), fb.height()), PixelRect{32, 32});
    // Set up PRNGs to start at different points in the period.
    for (auto &tileInfo : tiling) {
        tileInfo.randomGen = cloneForThread(rootRng, tileInfo.tileNumber);
    }
    me_->progress.tilesTarget = tiling.size();
    me_->progress.primaryRaysTarget = fb.width() * fb.height() * me_->options.samplesAA;

    tbb::task_group renderTaskGroup;

    auto taskStatus = renderTaskGroup.run_and_wait([&] {
        tbb::parallel_for_each(
            std::begin(tiling), std::end(tiling), [&](TileInfo &tileInfo) -> void {
                auto threadName = fmt::format("tile thread {}", tileInfo.tileNumber);
                loguru::set_thread_name(threadName.c_str());

                integrateTile(tileInfo, me_->options, me_->scene, fb);
                me_->progress.tilesCompleted++;
                me_->progress.primayRaysTraced += tileInfo.bounds.area() * me_->options.samplesAA;
                if (onProgress({}, RenderStatus::Running) != RenderCommand::Continue) {
                    tbb::task::self().cancel_group_execution();
                }
                auto percentComplete = 100.0f * static_cast<float>(me_->progress.tilesCompleted) /
                                       me_->progress.tilesTarget;
                if (static_cast<int>(percentComplete * 10) % 5 == 0)
                    LOG_F(INFO, "{:1.1f}% done..", percentComplete);
            });
    });

    if (taskStatus == tbb::canceled)
        LOG_F(WARNING, "Render was aborted.");
    onProgress({}, (taskStatus == tbb::canceled) ? RenderStatus::Aborted : RenderStatus::Running);

    // LOG_F(INFO, "Render took {} s", renderTimer.elapsed());
    LOG_F(INFO, "Saving image.");
    saveImage(fb);
}
} // namespace cornelis