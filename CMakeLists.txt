cmake_minimum_required(VERSION 3.21)

project(CompMathPrjc LANGUAGES C CXX)

# Defining standarts
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(COMPILE_FLAGS -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror)

include(./include_sofa.cmake)

include_directories(lib)
add_subdirectory(src)
