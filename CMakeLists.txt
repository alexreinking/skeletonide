cmake_minimum_required(VERSION 3.16)
project(skeletonide)

find_package(Halide REQUIRED)

add_executable(skeletongen src/pipeline.cpp)
target_link_libraries(skeletongen PRIVATE Halide::Generator)

if (WIN32)
    set(targets cmake-d3d12compute cmake-cuda cmake-opencl)
elseif (APPLE)
    set(targets cmake-metal)
else ()
    set(targets cmake-cuda cmake-opencl)
endif ()

add_halide_library(skeletonide FROM skeletongen
                   FUNCTION_NAME skel
                   TARGETS ${targets})

add_executable(spook spook.cpp)
target_link_libraries(spook PRIVATE skeletonide Halide::ImageIO)
