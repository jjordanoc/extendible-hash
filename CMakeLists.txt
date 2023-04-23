cmake_minimum_required(VERSION 3.25)
project(extendible_hash)

set(CMAKE_CXX_STANDARD 17)

add_executable(extendible_hash main.cpp ExtendibleHash.hpp)

add_executable(read_data read_data.cpp)

add_executable(misc_testing misc_testing.cpp)
