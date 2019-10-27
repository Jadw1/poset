cmake_minimum_required(VERSION 3.14)
project(poset)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

add_executable(poset poset.cc poset.h)