cmake_minimum_required(VERSION 3.10)
project(microui)
set(CMAKE_CXX_STANDARD 11)

add_library(microui STATIC src/microui.cpp)
target_include_directories(microui PUBLIC src/)
