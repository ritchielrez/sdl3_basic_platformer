cmake_minimum_required(VERSION 3.10)
project(imgui_impl_sdlrenderer3)
set(CMAKE_CXX_STANDARD 11)

add_library(imgui_impl_sdlrenderer3 STATIC
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    backends/imgui_impl_sdl3.cpp
    backends/imgui_impl_sdlrenderer3.cpp
)
target_include_directories(imgui_impl_sdlrenderer3 PRIVATE ../SDL/include/)
target_include_directories(imgui_impl_sdlrenderer3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(imgui_impl_sdlrenderer3 PUBLIC backends/)
