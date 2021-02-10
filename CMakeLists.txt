cmake_minimum_required(VERSION 3.16)
project(skeletonide)

enable_testing()

find_package(Halide REQUIRED)

add_executable(skeletongen src/pipeline.cpp)
target_link_libraries(skeletongen PRIVATE Halide::Generator)

if (WIN32)
    set(targets host-cuda host-d3d12compute host-opencl)
elseif (APPLE)
    set(targets host-metal)
else ()
    set(targets host-cuda host-opencl)
endif ()

add_halide_library(skeletonide FROM skeletongen
                   FUNCTION_NAME skel
                   TARGETS ${targets})

add_executable(spook spook.cpp)
target_link_libraries(spook PRIVATE skeletonide Halide::ImageIO)

add_test(NAME horse
         COMMAND spook ${CMAKE_CURRENT_SOURCE_DIR}/test/images/horse.png ${CMAKE_CURRENT_BINARY_DIR}/out.png)
