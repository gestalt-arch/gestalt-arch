cmake_minimum_required(VERSION 3.1)
project(lidar_classification)

# Append _d to the debug version
set(CMAKE_DEBUG_POSTFIX "_d")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(lidar_classification lidar_classification.c)