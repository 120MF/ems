cmake_minimum_required(VERSION 3.28)
project(ems CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)

option(EMS_BUILD_EXAMPLES "Build EMS examples" OFF)

add_library(ems INTERFACE)
target_include_directories(ems INTERFACE include)

if (EMS_BUILD_EXAMPLES)
    include(FetchContent)
    include(Modules/FindMiniaudio)
    add_subdirectory(example)
endif ()