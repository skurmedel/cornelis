#include <cornelis/Expects.hpp>
#include <cornelis/Scene.hpp>

namespace cornelis {
struct Scene::State {
    State() : activeCamera{std::make_shared<PerspectiveCamera>()} {}

    PerspectiveCameraPtr activeCamera;
    SurfaceBag<SphereSurface> spheres;
};

Scene::Scene() : me_(std::make_unique<State>()) {}
Scene::~Scene() {}

auto Scene::setCamera(PerspectiveCameraPtr camera) -> void {
    CORNELIS_EXPECTS(camera.get() != nullptr, "must set a proper camera.");
    me_->activeCamera = camera;
}
auto Scene::camera() const noexcept -> PerspectiveCameraPtr { return me_->activeCamera; }

auto Scene::spheres() noexcept -> SurfaceBag<SphereSurface> & { return me_->spheres; }
auto Scene::spheres() const noexcept -> SurfaceBag<SphereSurface> const & { return me_->spheres; }

SphereData::SphereData(span<const SphereDescription> descriptions)
    : SoAObject(descriptions.size()) {
    auto [x, y, z] = getPositions(*this);
    auto radius = get<tags::Radius>();
    auto materialId = get<tags::MaterialId>();
    for (std::size_t i = 0; i != descriptions.size(); i++) {
        auto const &descr = descriptions[i];
        x[i] = descr.center[0];
        y[i] = descr.center[1];
        z[i] = descr.center[2];
        radius[i] = descr.radius;
        materialId[i] = descr.material.value_or(0);
    }
}

PlaneData::PlaneData(span<const PlaneDescription> descriptions) : SoAObject(descriptions.size()) {
    auto [x, y, z] = getPositions(*this);
    auto [Nx, Ny, Nz] = getNormalSpans(*this);
    auto materialId = get<tags::MaterialId>();
    for (std::size_t i = 0; i != descriptions.size(); i++) {
        auto const &descr = descriptions[i];
        x[i] = descr.point[0];
        y[i] = descr.point[1];
        z[i] = descr.point[2];
        Nx[i] = descr.normal[0];
        Ny[i] = descr.normal[1];
        Nz[i] = descr.normal[2];
        materialId[i] = descr.material.value_or(0);
    }
}

SceneData::SceneData(SceneDescription const &descr)
    : camera{PerspectiveCamera::lookAt(descr.camera().origin,
                                       descr.camera().lookAt,
                                       descr.camera().aspect,
                                       descr.camera().horizontalFov)},
      materials{}, spheres{descr.spheres()}, planes{descr.planes()} {
    for (auto &matDescr : descr.materials()) {
        materials.push_back(StandardMaterial(matDescr.albedo, matDescr.emissive));
    }
}

} // namespace cornelis
