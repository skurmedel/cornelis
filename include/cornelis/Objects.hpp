#pragma once

#include <concepts>
#include <span>
#include <vector>

#include <cornelis/Materials.hpp>
#include <cornelis/Math.hpp>

/**
 * The types in this "module", might look a bit strange if you are used to standard OO.
 *
 * There's two things we try to achieve here:
 * We wish to avoid dynamic polymorphism through vtables, which is cumbersome when dealing with
 * GPU-CPU communication. We wish to achive a data oriented design, so that we can process a large
 * amount of objects quickly.
 *
 * This is not primarily done for speed, although I imagine in some cases it helps.
 */

namespace cornelis {

/**
 * Describes where a ray intersected an object.
 */
struct SurfaceHitInfo {
    float t0;
    V3 P;
    V3 N;
};

/**
 * Describes an object that can intersect with a ray.
 *
 * Every Object should support the Surface concept.
 */
template <typename T>
concept Surface = requires(T const &h, Ray const &r, SurfaceHitInfo &info) {
    // Supports intersection queries.
    // Method required: intersects(Ray const &r, SurfaceHitInfo &info) const -> bool
    { h.intersects(r, info) } -> std::same_as<bool>;
    // Compute the world space bounds.
    // Method required: worldBounds() const -> BBox
    // TODO: this needs to take in the transform.
    { h.worldBounds() } -> std::same_as<BBox>;
    // Construction requirements.
    std::copy_constructible<T>;
};

// TODO: this name is weird and strange.
template <Surface T>
class SurfaceBag {
  public:
    using surface_type = T;

    template <typename U>
    using container_type = std::vector<U>;

    auto add(surface_type const &geo, StandardMaterial const &mat, Transform xform = Transform{})
        -> void {
        bounds_.emplace_back(geo.worldBounds());
        transforms_.emplace_back(xform);
        geometries_.emplace_back(geo);
        materials_.emplace_back(mat);
    }

    /** Map to precomputed world bounds for the Surface at k. */
    auto bound(std::size_t k) const -> BBox const & { return bounds_[k]; }
    /** Map to world-object transform for the Surface at k. */
    auto transform(std::size_t k) const -> Transform const & { return transforms_[k]; }
    /** Map to surface description for the Surface at k. */
    auto geometry(std::size_t k) const -> surface_type const & { return geometries_[k]; }
    /** Map to materials for the Surface at k. */
    auto material(std::size_t k) const -> StandardMaterial const & { return materials_[k]; }

    auto bounds() -> std::span<BBox> { return bounds_; }
    auto bounds() const -> std::span<BBox const> { return bounds_; }
    auto transforms() -> std::span<Transform> { return transforms_; }
    auto transforms() const -> std::span<Transform const> { return transforms_; }
    auto geometries() -> std::span<surface_type> { return geometries_; }
    auto geometries() const -> std::span<surface_type const> { return geometries_; }
    auto materials() -> std::span<StandardMaterial> { return materials_; }
    auto materials() const -> std::span<StandardMaterial const> { return materials_; }

    auto size() const noexcept -> std::size_t { return bounds_.size(); }

  private:
    container_type<BBox> bounds_;
    container_type<Transform> transforms_;
    container_type<surface_type> geometries_;
    container_type<StandardMaterial> materials_;
};

struct SphereSurface {
    // TODO: test me.
    auto intersects(Ray const &ray, SurfaceHitInfo &info) const -> bool {
        float t0, t1;
        if (ray.intersects(center, radius, t0, t1)) {
            info.P = ray(t0);
            info.N = (info.P - center).normalize();
            info.t0 = t0;
            return true;
        }
        return false;
    }

    auto worldBounds() const -> BBox {
        BBox computed(center - V3(radius), center + V3(radius));
        return computed;
    }

    // TODO: we should have this work entirely in object space, which means center is pointless.
    V3 center;
    float radius;
};
static_assert(Surface<SphereSurface>, "SphereSurface does not fulfill Surface.");

} // namespace cornelis
