#include <cornelis/Render.hpp>
#include <cornelis/SceneDescription.hpp>

using namespace cornelis;

auto cornellBox() -> SceneDescription {
    float sideLen = 555.0f;
    float sideLenHalf = 550.0f / 2.0f;

    SceneDescription sceneDescr;
    sceneDescr.setCamera(PerspectiveCameraDescription{.origin = V3(0, sideLenHalf, -1100),
                                                      .lookAt = V3(0, sideLenHalf, 0),
                                                      .aspect = 1.f,
                                                      .horizontalFov = 0.7f});

    auto red = sceneDescr.addMaterial(MaterialDescription{.albedo = RGB{.65f, .05f, .05f}});
    auto white = sceneDescr.addMaterial(MaterialDescription{.albedo = RGB{.73f, .73f, .73f}});
    auto green = sceneDescr.addMaterial(MaterialDescription{.albedo = RGB{.12, .45f, .15f}});
    auto gold =
        sceneDescr.addMaterial(MaterialDescription{.albedo = RGB::black(),
                                                   .emissive = RGB::black(),
                                                   .roughness = 0.01f,
                                                   .reflectionTint = RGB(0.916f, 0.61f, 0.0f),
                                                   .ior = 0.470});

    auto light = sceneDescr.addMaterial(
        MaterialDescription{.albedo = RGB::black(), .emissive = RGB{15, 15, 15}});

    PlaneDescription leftWall{.normal = V3(1.0f, 0, 0),
                              .point = V3(-sideLenHalf, sideLenHalf, 0),
                              .extents = V3(sideLen, sideLen, 0)};
    leftWall.material = green;

    PlaneDescription rightWall{.normal = V3(-1.0f, 0, 0),
                               .point = V3(sideLenHalf, sideLenHalf, 0),
                               .extents = V3(sideLen, sideLen, 0)};
    rightWall.material = red;

    PlaneDescription roof{
        .normal = V3(0, -1.0f, 0), .point = V3(0, sideLen, 0), .extents = V3(sideLen, sideLen, 0)};
    roof.material = white;

    PlaneDescription floor{
        .normal = V3(0, 1.0f, 0), .point = V3(0, 0, 0), .extents = V3(sideLen, sideLen, 0)};
    floor.material = white;

    PlaneDescription backWall{.normal = V3(0, 0, -1.0f),
                              .point = V3(0, sideLenHalf, sideLenHalf),
                              .extents = V3(sideLen, sideLen, 0)};
    backWall.material = white;

    sceneDescr.addPlane(leftWall);
    sceneDescr.addPlane(rightWall);
    sceneDescr.addPlane(roof);
    sceneDescr.addPlane(floor);
    sceneDescr.addPlane(backWall);

    SphereDescription lightSphere{.center = V3(0, sideLen - 60.0f, 0), .radius = 60.0f};
    lightSphere.material = light;

    SphereDescription sphere2{.center = V3(0, 50.0f, 0), .radius = 50.0f};
    sphere2.material = red;
    SphereDescription sphere3{.center = V3(-160, 100.0f, 0), .radius = 100.0f};
    sphere3.material = white;
    SphereDescription sphere4{.center = V3(160, 125.0f, 200), .radius = 125.0f};
    sphere4.material = gold;

    sceneDescr.addSphere(lightSphere);
    sceneDescr.addSphere(sphere2);
    sceneDescr.addSphere(sphere3);
    sceneDescr.addSphere(sphere4);

    return sceneDescr;
}

int main(int argc, char *argv[]) {
    RenderSession session(cornellBox(), RenderOptions{.samplesAA = 10000});
    session.render();
}