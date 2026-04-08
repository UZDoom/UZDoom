cmake_minimum_required(VERSION 3.10)
project(nativefiledialog-extended VERSION 1.3.0)

set(nfd_ROOT_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(nfd_ROOT_PROJECT ON)
endif ()

option(BUILD_SHARED_LIBS "Build a shared library instead of static" OFF)
option(NFD_BUILD_TESTS "Build tests for nfd" ${nfd_ROOT_PROJECT})
option(NFD_BUILD_SDL2_TESTS "Build SDL2 tests for nfd" OFF)
option(NFD_BUILD_GLFW3_TESTS "Build GLFW3 tests for nfd" OFF)
option(NFD_INSTALL "Generate install target for nfd" ${nfd_ROOT_PROJECT})

set(nfd_PLATFORM Undefined)
if(WIN32)
  set(nfd_PLATFORM PLATFORM_WIN32)
elseif(APPLE)
  set(nfd_PLATFORM PLATFORM_MACOS)
elseif(UNIX AND NOT APPLE)
  set(nfd_PLATFORM PLATFORM_LINUX)
endif()

message("nfd Platform: ${nfd_PLATFORM}")

set(nfd_COMPILER Undefined)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
  # This is clang-cl, which has different compiler options
  set(nfd_COMPILER COMPILER_CLANGCL)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(nfd_COMPILER COMPILER_MSVC)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  set(nfd_COMPILER COMPILER_GNU)
endif()

message("nfd Compiler: ${nfd_COMPILER}")

# Use latest C++ by default (should be the best one), but let user override it
if(NOT DEFINED CMAKE_CXX_STANDARD)
  if(CMAKE_VERSION VERSION_LESS "3.12")
    set (CMAKE_CXX_STANDARD 17)
  elseif(CMAKE_VERSION VERSION_LESS "3.20")
    set (CMAKE_CXX_STANDARD 20)
  else()
    set (CMAKE_CXX_STANDARD 23)
  endif()
endif()

add_subdirectory(src)

if(${NFD_BUILD_TESTS} OR ${NFD_BUILD_SDL2_TESTS} OR ${NFD_BUILD_GLFW3_TESTS})
  add_subdirectory(test)
endif()
