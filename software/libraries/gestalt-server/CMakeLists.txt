# CMake minimum version
cmake_minimum_required(VERSION 3.1)

# Cmake file for making the MTIDevice library
project( gestalt-server )

# Append _d to the debug version of MTIDevice => libMTIDevice_d.so
set(CMAKE_DEBUG_POSTFIX "_d")


# Specifies a library name, specifies whether the library is STATIC or
# SHARED, and provides relative paths to the source code. You can
# define multiple libraries by adding multiple add_library() commands,
# and CMake builds them for you
add_library(
    # Library name
    gestalt-server

    # Library type (SHARED or STATIC)
    SHARED

    # Source files
    gestalt-server.c )
