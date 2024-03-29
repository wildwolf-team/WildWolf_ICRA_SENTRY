cmake_minimum_required(VERSION 2.6)

project(main)

add_definitions(-std=c++17)
add_definitions(-DAPI_EXPORTS)
option(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

# cuda
find_package(CUDA REQUIRED)
include_directories(/usr/local/cuda/include)
link_directories(/usr/local/cuda/lib64)

# find cppzmq wrapper, installed by make of cppzmq
find_package(cppzmq)

find_package(Threads)

include_directories(.)
include_directories(./TRTX)
include_directories(./camera)

add_subdirectory(TRTX)
add_subdirectory(camera)


add_executable(main yolov5main)
target_link_libraries(main yolov5)    # fmt::fmt
target_link_libraries(main mv_video_capture)
target_link_libraries(main cppzmq)
target_link_libraries(main ${CMAKE_THREAD_LIBS_INIT})
