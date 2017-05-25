cmake_minimum_required(VERSION 2.8)
project(rubik-cube-solver)

find_package(OpenCV REQUIRED)

set(SRC_FILES src/main.cpp src/rubik_cube.cpp)

add_executable(rubik-cube-solver ${SRC_FILES})

target_compile_options(rubik-cube-solver PUBLIC -std=c++1y)
