cmake_minimum_required(VERSION 3.10)
project(Alternative_Parallel)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp")
set(CMAKE_CXX_STANDARD 11)

add_executable(Alternative_Parallel main.cpp)