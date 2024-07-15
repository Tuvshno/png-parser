cmake_minimum_required(VERSION 3.28)
project(PNGParser)

set(CMAKE_CXX_STANDARD 17)

add_executable(PNGParser main.cpp
        memory_utils.h
        memory_utils.cpp
        png_utils.cpp
        png_utils.h)
