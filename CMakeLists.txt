cmake_minimum_required(VERSION 3.10)
project(sdl3_basic_platformer)
set(CMAKE_CXX_STANDARD 20)

set(BUILD_SHARED_LIBS off)
set(SDL_STATIC on)
add_subdirectory(vendor/SDL)
add_subdirectory(vendor/stb_image)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/fmt)
add_subdirectory(vendor/imgui)

if(MSVC)
  if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
      add_definitions(/MP)				# parallelize each target, unless Ninja is the generator
  endif()
endif()

add_executable(main src/main.cpp src/Entity.cpp src/Player.cpp)
target_include_directories(main PRIVATE include)

if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  target_compile_definitions(main PRIVATE DEBUG)
endif()

if(WIN32)
  target_compile_definitions(main PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

target_link_libraries(main PUBLIC $<$<BOOL:${MINGW}>:ws2_32>)
target_link_libraries(main PUBLIC SDL3::SDL3-static)
target_link_libraries(main PUBLIC glm::glm)
target_link_libraries(main PUBLIC fmt::fmt)
target_link_libraries(main PUBLIC stb_image)
target_link_libraries(main PUBLIC imgui_impl_sdlrenderer3)
