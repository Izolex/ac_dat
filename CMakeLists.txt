cmake_minimum_required(VERSION 3.21)
project(text_search C)

set(CMAKE_C_STANDARD 99)

add_executable(text_search src/main.c src/char_coding.c src/dat.c src/tail.c src/debug.c)
