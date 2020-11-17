cmake_minimum_required(VERSION 3.10)
project(blender-draco-exporter)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/2.80/python/lib/python3.7)

add_subdirectory(draco EXCLUDE_FROM_ALL)

add_library(${PROJECT_NAME} SHARED src/encoder.cpp src/encoder.h)
target_include_directories(${PROJECT_NAME} PUBLIC draco/src)
target_link_libraries(${PROJECT_NAME} PUBLIC draco)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

