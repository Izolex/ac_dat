cmake_minimum_required(VERSION 3.21)
set(CMAKE_C_STANDARD 99)
project(ac_dat_container C)


find_package(PkgConfig REQUIRED)
pkg_search_module(EVENT REQUIRED IMPORTED_TARGET libevent)


if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DVERBOSE=1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
        -fsanitize=address -fsanitize=undefined -g3 -Wall -Wextra -Werror -O0 \
        -Wimplicit-function-declaration -Wimplicit-int -Wold-style-definition \
        -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes -Wout-of-line-declaration \
    ")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
endif()


add_subdirectory(bench)
add_subdirectory(lib)
add_subdirectory(example)
add_subdirectory(cmd)
