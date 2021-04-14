cmake_minimum_required(VERSION 2.8)

project("oclparticles")

set(exec "main")
set(target_path ${CMAKE_SOURCE_DIR})

file(GLOB target_cpp ${target_path}/*.cpp ${target_path}/*.h)

set(CMAKE_BUILD_TYPE Debug)
add_definitions(-std=c++11 -Wall)

add_executable(${exec} ${target_cpp})
