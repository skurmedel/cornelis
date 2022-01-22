include(FetchContent)

FetchContent_Declare(
  catch
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.0-preview3
)

FetchContent_MakeAvailable(catch)

add_executable(cornelis_test_runner
    test_FrameBuffer.cpp
    test_Color.cpp
    test_Camera.cpp
    test_Scene.cpp
    test_Sampling.cpp
    test_Tiles.cpp
    test_Math.cpp
    test_Linalg.cpp
    test_SoA.cpp
    test_SceneDescription.cpp
    test_Geometry.cpp
)
target_link_libraries(cornelis_test_runner PUBLIC corneliscore Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH "${catch_SOURCE_DIR}/extras")
message(${CMAKE_MODULE_PATH})

include(CTest)
include(Catch)
catch_discover_tests(cornelis_test_runner)