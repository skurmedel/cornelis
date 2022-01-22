find_package(Threads REQUIRED)

add_library(loguru STATIC
    loguru-2.1.0/loguru.cpp
)

target_link_libraries(loguru PUBLIC dl Threads::Threads fmt::fmt)
target_include_directories(loguru PUBLIC loguru-2.1.0)
set_target_properties(loguru PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(loguru PUBLIC LOGURU_USE_FMTLIB=1)