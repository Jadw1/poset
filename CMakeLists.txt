cmake_minimum_required(VERSION 3.14)
project(poset)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++17 -O2")

add_executable(poset poset.cc poset.h poset_example1.c)