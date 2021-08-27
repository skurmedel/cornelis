project(cornelis)

cmake_minimum_required(VERSION 3.16)

find_package(TBB CONFIG REQUIRED)

macro(cornelis_all_warnings TARGET)
    if(MSVC)
    target_compile_options(${TARGET} PRIVATE /W4 /WX)
    else()
    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endmacro()

add_subdirectory(src)
add_subdirectory(tests)