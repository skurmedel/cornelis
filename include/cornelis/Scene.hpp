#pragma once

#include <memory>

#include <cornelis/Camera.hpp>
#include <cornelis/Objects.hpp>

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
} // namespace cornelis
