#pragma once

#include <cornelis/Expects.hpp>
#include <cornelis/Math.hpp>

namespace cornelis {
/**
 * A simple idealized perspective camera, where, in camera space, positive Z is the camera axis
 * towards the subject, negative X points to the left and positive Y is up in the view.
 *
 * Does not support bokeh or distortion at the moment, nor film back offset.
 */
class PerspectiveCamera {
  public:
    PerspectiveCamera();

    PerspectiveCamera(PerspectiveCamera const &) = default;
    PerspectiveCamera(PerspectiveCamera &&) = default;

    /**
     * Creates a camera ray for the given location on the sensor/film plane.
     *
     * The camera ray is in world space.
     *
     * x values in [0, 1] denote a horizontal position on the film.
     * y values in [0, 1] denote a vertical position on the film.
     */
    auto operator()(float x, float y) const noexcept -> Ray;

    /**
     * Creates a new camera at a given position, looking towards some target.
     *
     * aspectRatio is the width to height ratio, for example 3:2 = 1.5.
     *
     * hFov is the horizontal field of view in radians. It should be a value in [0, pi]. Values
     * outside this range will yield a flipped image and/or severe distortion.
     */
    static auto lookAt(V3 const &from, V3 const &at, float aspectRatio, float hFov)
        -> PerspectiveCamera;

  private:
    V3 eye_;
    V3 corner_;
    V3 u_;
    V3 v_;
};

/// Corresponds to something like a 43 mm lens for a 35 mm camera.
inline static constexpr float HorizontalFovNormal = 1.011f;

/**
 * Calculates the theoretical horizontal field of view for a 35 mm lens with a given focal length.
 *
 * \pre focalLength > 0
 */
auto horizontalFov35mm(float focalLength) -> float;
} // namespace cornelis
