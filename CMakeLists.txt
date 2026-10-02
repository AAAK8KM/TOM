cmake_minimum_required(VERSION 3.21)

project(CompMathPrjc LANGUAGES C CXX)

# Defining standarts
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(COMPILE_FLAGS -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror)

include(./include_sofa.cmake)
include(./include_eigen.cmake)
include(./include_calceph.cmake)

option(ORBMECH_WITH_IERS "Build the libcurl IERS Bulletin B adapter" OFF)

enable_testing()
include(./include_gtest.cmake)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(tests)

if(ORBMECH_WITH_IERS)
  add_subdirectory(adapter)
endif()
