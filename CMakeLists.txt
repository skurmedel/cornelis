project(cornelis)

cmake_minimum_required(VERSION 3.16)

find_package(TBB CONFIG REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)