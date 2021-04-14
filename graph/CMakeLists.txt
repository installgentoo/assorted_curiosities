cmake_minimum_required(VERSION 3.1)
project("demo")

include(${CMAKE_SOURCE_DIR}/../.cmake/util.cmake)

add_module(FreeType)
add_module(glm)
add_module(lodepng)
add_module(gl3w)
add_module(glfw)
add_module(opengl)
add_module(std_thread)

copy_resources(${CMAKE_SOURCE_DIR}/resources)

set(CMAKE_CXX_STANDARD 11)
set_debug()
#set_release()

set(BASE_DIR ${CMAKE_SOURCE_DIR}/base_classes)
set(GL_GUI "glgui")
build_library(${GL_GUI} ${CMAKE_SOURCE_DIR}/gui ${BASE_DIR})
link_directories(${CMAKE_BINARY_DIR}/lib)

build_exec(${CMAKE_SOURCE_DIR}/demo ${GL_GUI})
