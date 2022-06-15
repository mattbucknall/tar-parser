cmake_minimum_required(VERSION 3.22)
project(tar-parser C)

set(CMAKE_C_STANDARD 99)

add_executable(tar-parser main.c tar.c)
target_compile_definitions(tar-parser PRIVATE -DTAR_DEBUG)
