# CMake minimum version
cmake_minimum_required(VERSION 3.1)

# Cmake file for making the gestalt-client library
project( gestalt-client )

# Append _d to the debug version
set(CMAKE_DEBUG_POSTFIX "_d")


# Specifies a library name, specifies whether the library is STATIC or
# SHARED, and provides relative paths to the source code. You can
# define multiple libraries by adding multiple add_library() commands,
# and CMake builds them for you
add_library(
    # Library name
    gestalt-client

    # Library type (SHARED or STATIC)
    SHARED

    # Source files
    gestalt-client.c )
