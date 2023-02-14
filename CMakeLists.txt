project( computeoveraccess )
cmake_minimum_required(VERSION 3.18.0)

enable_language( CXX )
#include(FetchContent)

set (CMAKE_CXX_FLAGS "-std=c++17")

add_executable(bench bench.cpp)
