project(topper CXX)

# Required CMake version
cmake_minimum_required(VERSION 2.8)

# Module search path
set(CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/cmake
)

# Some CMake configuration
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

# Set up the install prefix
set(topper_INSTALL_PREFIX "${topper_BINARY_DIR}/prefix")

# Default to release builds w/ debug info
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif (NOT CMAKE_BUILD_TYPE)

# Add third-party dependencies
include(ExternalProject)
include(External_gtest)
include(External_Boost)
include(External_http-parser)
include(External_glog)
include(External_wte)
include(External_ccmetrics)

# Use C++11
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Recurse
add_subdirectory(include)
add_subdirectory(example)
add_subdirectory(src)
add_subdirectory(test)
