cmake_minimum_required(VERSION 3.24)
project(rename)

set(CMAKE_CXX_STANDARD 23)

if (CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install directory: ${CMAKE_INSTALL_BINDIR}")

add_subdirectory(src)
