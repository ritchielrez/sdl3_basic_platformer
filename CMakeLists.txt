cmake_minimum_required(VERSION 3.10)
project(sdl3_basic_platformer)
set(CMAKE_CXX_STANDARD 20)

set(SDL3_URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.0/SDL3-devel-3.4.0-)
set(SDL3_IMAGE_URL https://github.com/libsdl-org/SDL_image/releases/download/release-3.4.0/SDL3_image-devel-3.4.0-)

if(MSVC)
  string(APPEND SDL3_URL VC.zip)
  string(APPEND SDL3_IMAGE_URL VC.zip)
elseif(MINGW)
  string(APPEND SDL3_URL mingw.zip)
  string(APPEND SDL3_IMAGE_URL mingw.zip)
else()
  message(FATAL_ERROR "This platform is not support yet.")
endif()

include(FetchContent)
FetchContent_Populate(
  sdl3
  URL ${SDL3_URL}
)
FetchContent_Populate(
  sdl3_image
  URL ${SDL3_IMAGE_URL}
)

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

list(APPEND CMAKE_PREFIX_PATH "${sdl3_SOURCE_DIR}/cmake")
list(APPEND CMAKE_PREFIX_PATH "${sdl3_image_SOURCE_DIR}/cmake")

find_package(SDL3)
find_package(SDL3_image)

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
target_link_libraries(main PUBLIC SDL3::SDL3)
target_link_libraries(main PUBLIC SDL3_image::SDL3_image)
target_link_libraries(main PUBLIC glm::glm)
target_link_libraries(main PUBLIC fmt::fmt)

if(WIN32 AND CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "AMD64")
  set(SDL3_DLL ${sdl3_SOURCE_DIR}/lib/x64/SDL3.dll)
  set(SDL3_IMAGE_DLL ${sdl3_image_SOURCE_DIR}/lib/x64/SDL3_image.dll)

  add_custom_command(TARGET main POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy ${SDL3_DLL} ${CMAKE_BINARY_DIR})
  add_custom_command(TARGET main POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy ${SDL3_IMAGE_DLL} ${CMAKE_BINARY_DIR})
endif()
