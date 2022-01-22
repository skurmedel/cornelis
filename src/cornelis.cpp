#include <cornelis/Render.hpp>
#include <cornelis/SceneDescription.hpp>

using namespace cornelis;

int main(int argc, char *argv[]) {
    SceneDescription sceneDescr;
    sceneDescr.setCamera(
        PerspectiveCameraDescription{.origin = V3(0, 2, -5), .lookAt = V3(0), .aspect = 0.5f});

    auto mat1 = sceneDescr.addMaterial(MaterialDescription{.albedo = RGB::red()});
    auto mat2 = sceneDescr.addMaterial(MaterialDescription{.albedo = RGB(0.9, 0.9, 0.9)});
    auto mat3 = sceneDescr.addMaterial(
        MaterialDescription{.albedo = RGB::black(), .emissive = RGB(1.0, 1.0, 1.0) * 50.0f});

    SphereDescription sphere1{.center = V3(0, 0.5f, 0), .radius = 1.0f};
    sphere1.material = mat1;
    SphereDescription sphere2{.center = V3(0, -100.0f, 0), .radius = 100.0f};
    sphere2.material = mat2;
    // A light.
    SphereDescription sphere3{.center = V3(50, 50, -50), .radius = 7.0f};
    sphere3.material = mat3;

    sceneDescr.addSphere(sphere1);
    sceneDescr.addSphere(sphere2);
    sceneDescr.addSphere(sphere3);

    RenderSession session(sceneDescr, RenderOptions{});
    session.render();
}