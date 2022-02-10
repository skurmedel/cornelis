#pragma once

#include <optional>
#include <vector>

#include <cornelis/Color.hpp>
#include <cornelis/Span.hpp>

namespace cornelis {
/*struct V3 {
    float values[3];
};*/

struct MaterialDescription {
    RGB albedo = RGB(0.5, 0.5, 0.5);
    RGB emissive = RGB::black();
    float roughness = 0.2f;
    RGB reflectionTint = RGB::black();
    float ior = 1.5f;

    auto operator==(MaterialDescription const &obj) const -> bool = default;
};

struct ObjectDescription {
    std::optional<std::size_t> material;

    auto operator==(ObjectDescription const &obj) const -> bool = default;
};

struct SphereDescription : public ObjectDescription {
    V3 center;
    float radius = 1.0f;

    auto operator==(SphereDescription const &) const -> bool = default;
};

struct PlaneDescription : public ObjectDescription {
    V3 normal{0, 1, 0};
    V3 point{0, 0, 0};
    V3 extents{1000.f, 1000.f, 0.0f};

    auto operator==(PlaneDescription const &) const -> bool = default;
};

struct PerspectiveCameraDescription : public ObjectDescription {
    V3 origin;
    V3 lookAt{0, 0, 1};
    float aspect = 0.5f;
    // TODO: Make this into some kind of "safe-type" for positive only values.
    float horizontalFov = 1.011f; // Horizontal FOV for a 35 mm camera "normal" (43 mm).

    auto operator==(PerspectiveCameraDescription const &) const -> bool = default;
};

class SceneDescription {
  public:
    SceneDescription() = default;

    auto setCamera(PerspectiveCameraDescription const &cam) -> void { camera_ = cam; }

    auto addMaterial(MaterialDescription const &mat) -> std::size_t {
        materials_.push_back(mat);
        return materials_.size() - 1;
    }

    auto addSphere(SphereDescription const &sphere) -> std::size_t {
        spheres_.push_back(sphere);
        return spheres_.size() - 1;
    }

    auto addPlane(PlaneDescription const &plane) -> std::size_t {
        planes_.push_back(plane);
        return planes_.size() - 1;
    }

    auto materials() const noexcept -> span<const MaterialDescription> { return materials_; }

    auto spheres() const noexcept -> span<const SphereDescription> { return spheres_; }

    auto planes() const noexcept -> span<const PlaneDescription> { return planes_; }

    auto camera() const noexcept -> PerspectiveCameraDescription { return camera_; }

  private:
    template <typename T>
    using container_type = std::vector<T>;

    PerspectiveCameraDescription camera_;
    container_type<MaterialDescription> materials_ = {MaterialDescription{}};
    container_type<SphereDescription> spheres_;
    container_type<PlaneDescription> planes_;
};

} // namespace cornelis
