add_library(corneliscore STATIC
    FrameBuffer.cpp
    NanoVDBMath.cpp
    Color.cpp

    extern/stb_image_write.cpp
)
target_compile_features(corneliscore PUBLIC cxx_std_20)
target_include_directories(corneliscore PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_include_directories(corneliscore PUBLIC "${PROJECT_SOURCE_DIR}/external")

add_executable(cornelis
    cornelis.cpp)
target_link_libraries(cornelis PRIVATE corneliscore )
target_compile_features(cornelis PUBLIC cxx_std_20)
target_include_directories(cornelis PRIVATE "${PROJECT_SOURCE_DIR}/external")