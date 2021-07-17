#include <cornelis/Camera.hpp>
#include <cornelis/Expects.hpp>

#include <cmath>

namespace cornelis {
PerspectiveCamera::PerspectiveCamera()
    : eye_(0), corner_(-0.4794255386, -0.4794255386, 1), u_(0.4794255386 * 2, 0, 0),
      v_(0, 0.4794255386 * 2, 0) {}

auto PerspectiveCamera::operator()(float x, float y) const noexcept -> Ray {
    return Ray(eye_, (corner_ + x * u_ + y * v_).normalize());
}

auto PerspectiveCamera::lookAt(V3 const &from, V3 const &at, float aspectRatio, float hFov)
    -> PerspectiveCamera {
    static V3 const up(0, 1, 0);

    V3 dir = (at - from).normalize();
    V3 u = dir.cross(up);
    V3 v = u.cross(dir);

    float fovScale = 2.0 * std::sin(hFov * 0.5);
    u *= fovScale;
    v *= aspectRatio * fovScale;

    PerspectiveCamera cam;
    cam.eye_ = from;
    cam.corner_ = dir - 0.5 * u - 0.5 * v;
    cam.u_ = u;
    cam.v_ = v;

    return cam;
}

auto horizontalFov35mm(float focalLength) -> float {
    CORNELIS_EXPECTS(focalLength > 0.0f, "Does not support zero or negative focal lengths.");
    return 2.0 * std::atan(36.0f / (2.0 * focalLength));
}

} // namespace cornelis
