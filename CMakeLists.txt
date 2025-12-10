cmake_minimum_required(VERSION 3.31)
project(ems CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(ems INTERFACE)
target_include_directories(ems INTERFACE include)

add_subdirectory(example)