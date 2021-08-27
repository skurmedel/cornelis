add_library(corneliscore STATIC
    FrameBuffer.cpp
    NanoVDBMath.cpp
    Color.cpp
    Camera.cpp
    Scene.cpp
    Render.cpp
    Random.cpp
    Tiles.cpp

    extern/stb_image_write.cpp
)
target_compile_features(corneliscore PUBLIC cxx_std_20)
target_include_directories(corneliscore PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_include_directories(corneliscore SYSTEM PUBLIC "${PROJECT_SOURCE_DIR}/external")
target_link_libraries(corneliscore PUBLIC tbb)

add_executable(cornelis
    cornelis.cpp)
target_link_libraries(cornelis PUBLIC corneliscore )
target_compile_features(cornelis PUBLIC cxx_std_20)
target_include_directories(cornelis PRIVATE "${PROJECT_SOURCE_DIR}/external")

cornelis_all_warnings(corneliscore)
cornelis_all_warnings(cornelis)