cmake_minimum_required(VERSION 3.16)
project(SoundVisualizer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(inc)

add_executable(SoundVisualizer
    src/main.cpp
)
