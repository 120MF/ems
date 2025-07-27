cmake_minimum_required(VERSION 3.31)
project(ems C)

set(CMAKE_C_STANDARD 11)

add_library(ems STATIC src/ems_parser.c)
target_include_directories(ems PUBLIC include)
