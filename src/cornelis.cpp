#include <cornelis/Render.hpp>

using namespace cornelis;

int main(int argc, char *argv[]) {
    auto camera = std::make_shared<PerspectiveCamera>(
        PerspectiveCamera::lookAt(V3(0, 0, 2), V3(0.0f), 0.5f, HorizontalFovNormal));

    Scene scene;
    scene.setCamera(camera);
    Ray r = (*scene.camera())(0.5, 0.5);
    printf("Camera Ray Center %f %f %f\n", r.eye()[0], r.eye()[1], r.eye()[2]);
    printf("Camera Ray Dir    %f %f %f\n", r.dir()[0], r.dir()[1], r.dir()[2]);

    RenderSession session(scene, RenderOptions{});
    session.render();
}