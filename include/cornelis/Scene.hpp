#pragma once

#include <memory>

#include <cornelis/Camera.hpp>
#include <cornelis/Materials.hpp>
#include <cornelis/Objects.hpp>
#include <cornelis/SceneDescription.hpp>
#include <cornelis/SoA.hpp>

namespace cornelis {
class Scene {
  public:
    Scene();
    ~Scene();
    Scene(Scene &&) = default;
    auto operator=(Scene &&) -> Scene & = default;

    Scene(Scene const &) = delete;
    auto operator=(Scene const &) -> Scene & = delete;

    auto setCamera(PerspectiveCameraPtr camera) -> void;
    auto camera() const noexcept -> PerspectiveCameraPtr;

    auto spheres() noexcept -> SurfaceBag<SphereSurface> &;
    auto spheres() const noexcept -> SurfaceBag<SphereSurface> const &;

  private:
    struct State;
    std::unique_ptr<State> me_;
};

namespace tags {
struct Radius {
    using element_type = float;
};
} // namespace tags

/**
 * Holds the data for all the Spheres in a Scene.
 */
struct SphereData
    : public SoAObject<tags::PositionX, tags::PositionY, tags::PositionZ, tags::Radius, tags::MaterialId> {
    SphereData(span<const SphereDescription> descriptions);
};

/**
 * Planes in a Point - Normal form.
 */
struct PlaneData : public SoAObject<tags::PositionX,
                             tags::PositionY,
                             tags::PositionZ,
                             tags::NormalX,
                             tags::NormalY,
                             tags::NormalZ,
                             tags::MaterialId> {
    PlaneData(span<const PlaneDescription> descriptions);
};

struct SceneData {
    SceneData(SceneDescription const &descr);

    PerspectiveCamera camera;

    std::vector<StandardMaterial> materials;
    SphereData spheres;
    PlaneData planes;
};
} // namespace cornelis
