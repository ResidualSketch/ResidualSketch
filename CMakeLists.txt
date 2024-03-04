cmake_minimum_required(VERSION 3.14)
project(EXEC)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O3 ")

include_directories(.)
include_directories(Common)
include_directories(Struct)
include_directories(Algorithm)

add_executable(EXEC main.cpp)
