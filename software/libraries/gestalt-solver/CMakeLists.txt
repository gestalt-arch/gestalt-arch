# CMake minimum version
cmake_minimum_required(VERSION 3.1)

# Cmake file for making the gestalt-solver library
project( gestalt-solver )

# Append _d to the debug version
set(CMAKE_DEBUG_POSTFIX "_d")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Specifies a library name, specifies whether the library is STATIC or
# SHARED, and provides relative paths to the source code. You can
# define multiple libraries by adding multiple add_library() commands,
# and CMake builds them for you
add_library(
    # Library name
    gestalt_solver

    # Library type (SHARED or STATIC)
    SHARED

    # Source files
    "gestalt-solver.cpp" )