
include(FetchContent)

FetchContent_Declare(
  xsimd
  GIT_REPOSITORY https://github.com/xtensor-stack/xsimd.git
  GIT_TAG        7.6.0
)

FetchContent_MakeAvailable(xsimd)


FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        8.1.0
)
FetchContent_MakeAvailable(fmtlib)


add_subdirectory(extern)


add_library(corneliscore STATIC
    FrameBuffer.cpp
    NanoVDBMath.cpp
    Color.cpp
    Camera.cpp
    Scene.cpp
    Render.cpp
    Random.cpp
    Tiles.cpp
    Linalg.cpp
    SoA.cpp
    SceneDescription.cpp
    Geometry.cpp
    Materials.cpp

    extern/stb_image_write.cpp
)
target_compile_features(corneliscore PUBLIC cxx_std_20)
target_include_directories(corneliscore PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_include_directories(corneliscore SYSTEM PUBLIC "${PROJECT_SOURCE_DIR}/external")
target_link_libraries(corneliscore PUBLIC xsimd)
target_link_libraries(corneliscore PUBLIC tbb)
target_link_libraries(corneliscore PUBLIC fmt::fmt loguru)

add_executable(cornelis
    cornelis.cpp)
target_link_libraries(cornelis PUBLIC corneliscore )
target_compile_features(cornelis PUBLIC cxx_std_20)
target_include_directories(cornelis PRIVATE "${PROJECT_SOURCE_DIR}/external")

cornelis_all_warnings(corneliscore)
cornelis_all_warnings(cornelis)
