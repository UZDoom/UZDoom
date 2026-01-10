cmake_minimum_required(VERSION 3.11)

project(Trex)

option(BUILD_EXAMPLES "Build examples" OFF)
option(BUILD_TESTS "Build tests" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT trex)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp" "include/*.hpp")
add_library(trex STATIC ${SOURCES})
target_include_directories(trex PUBLIC include/)
target_link_libraries(trex freetype harfbuzz)

####################
### Dependencies ###
####################

# FreeType
target_include_directories(trex PRIVATE ${freetype_SOURCE_DIR}/src)

# HarfBuzz
set(HB_BUILD_SUBSET OFF CACHE BOOL "" FORCE)

# stb
target_include_directories(trex PRIVATE "../../common/thirdparty/stb")

####################

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
