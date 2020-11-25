cmake_minimum_required(VERSION 3.10)
project(Blender_ExternDraco)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)

option(BUILD_FOR_GLTF "" ON)
add_subdirectory(draco EXCLUDE_FROM_ALL)

add_library(extern_draco SHARED src/encoder.cpp src/encoder.h src/decoder.cpp src/decoder.h src/common.cpp src/common.h)
target_include_directories(extern_draco PUBLIC draco/src ${CMAKE_BINARY_DIR})
target_link_libraries(extern_draco PUBLIC draco)
set_property(TARGET extern_draco PROPERTY POSITION_INDEPENDENT_CODE ON)

