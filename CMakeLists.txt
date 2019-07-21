cmake_minimum_required(VERSION 3.14)
project(Kaktoos)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_XX_FLAGS} -Wall -O3")
add_executable(Kaktoos main.cpp)