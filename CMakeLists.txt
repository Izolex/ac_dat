cmake_minimum_required(VERSION 3.21)
project(text_search C)

set(CMAKE_C_STANDARD 99)

add_executable(text_search src/main.c src/char.c src/mem.c src/dat.c src/tail.c src/ac.c src/list.c src/print.c src/mem.c src/file.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fsanitize=undefined -g -Wall -O0")
