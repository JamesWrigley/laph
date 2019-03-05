cmake_minimum_required(VERSION 3.8.0)

project(Laph)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 14)
add_definitions(-DJULIA_ENABLE_THREADING)

find_package(Qt5 COMPONENTS Widgets Qml Quick REQUIRED)

link_directories($ENV{JULIA_LIB_PATH})
include_directories($ENV{JULIA_INCLUDE_PATH})
add_compile_options(-Wall -Wextra -Wfatal-errors)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--export-dynamic")

add_subdirectory(src)
