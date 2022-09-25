cmake_minimum_required(VERSION 3.21)
project(ac_dat_example C)

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::EVENT libac_dat)
