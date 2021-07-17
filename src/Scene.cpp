#include <cornelis/Expects.hpp>
#include <cornelis/Scene.hpp>

namespace cornelis {
struct Scene::State {
    State() : activeCamera{std::make_shared<PerspectiveCamera>()} {}

    PerspectiveCameraPtr activeCamera;
};

Scene::Scene() : me_(std::make_unique<State>()) {}
Scene::~Scene() {}

auto Scene::setCamera(PerspectiveCameraPtr camera) -> void {
    CORNELIS_EXPECTS(camera.get() != nullptr, "must set a proper camera.");
    me_->activeCamera = camera;
}
auto Scene::camera() const noexcept -> PerspectiveCameraPtr { return me_->activeCamera; }

} // namespace cornelis
