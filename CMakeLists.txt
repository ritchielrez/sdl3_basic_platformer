cmake_minimum_required(VERSION 3.10)
project(sdl3_basic_platformer)
set(CMAKE_CXX_STANDARD 20)

FetchContent_Declare(
  glm
  GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	0af55ccecd98d4e5a8d1fad7de25ba429d60e863 #refs/tags/1.0.1
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 407c905e45ad75fc29bf0f9bb7c5c2fd3475976f #refs/tags/12.1.0
)
FetchContent_MakeAvailable(fmt)
set(SDL_STATIC on)
add_subdirectory(vendor/SDL)
add_subdirectory(vendor/stb_image)

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
