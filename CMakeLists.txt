cmake_minimum_required(VERSION 3.17)
project(comimant-server C)

set(CMAKE_C_STANDARD 99)

add_executable(comimant-server main.c main.h include/cjson/cJSON.c include/cjson/cJSON.h config.c config.h)
