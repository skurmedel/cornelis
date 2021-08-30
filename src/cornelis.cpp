#include <cornelis/Render.hpp>

using namespace cornelis;

int main(int argc, char *argv[]) {
    auto camera = std::make_shared<PerspectiveCamera>(
        PerspectiveCamera::lookAt(V3(0, 2, -5), V3(0.0f), 0.5f, HorizontalFovNormal));

    Scene scene;
    scene.setCamera(camera);
    Ray r = (*scene.camera())(0.5, 0.5);
    printf("Camera Ray Center %f %f %f\n", r.eye()[0], r.eye()[1], r.eye()[2]);
    printf("Camera Ray Dir    %f %f %f\n", r.dir()[0], r.dir()[1], r.dir()[2]);

    scene.spheres().add(SphereSurface{.center = V3(0, 0.5f, 0), .radius = 1.0f},
                        StandardMaterial{RGB::red(), RGB::black()});
    scene.spheres().add(SphereSurface{.center = V3(0, -100.0f, 0), .radius = 100.0f},
                        StandardMaterial{RGB(0.9, 0.9, 0.9), RGB::black()});
    // A "light".
    scene.spheres().add(SphereSurface{.center = V3(50, 50, -50), .radius = 7.0f},
                        StandardMaterial{RGB::black(), RGB(1.0, 1.0, 1.0) * 400.0f});

    RenderSession session(scene, RenderOptions{});
    session.render();
}