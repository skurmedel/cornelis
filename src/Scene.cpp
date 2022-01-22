#include <cornelis/Expects.hpp>
#include <cornelis/Scene.hpp>

namespace cornelis {
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
    auto widths = get<tags::WidthF>();
    auto heights = get<tags::HeightF>();
    auto materialId = get<tags::MaterialId>();
    for (std::size_t i = 0; i != descriptions.size(); i++) {
        auto const &descr = descriptions[i];
        x[i] = descr.point[0];
        y[i] = descr.point[1];
        z[i] = descr.point[2];
        Nx[i] = descr.normal[0];
        Ny[i] = descr.normal[1];
        Nz[i] = descr.normal[2];
        widths[i] = descr.extents[0];
        heights[i] = descr.extents[1];
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
