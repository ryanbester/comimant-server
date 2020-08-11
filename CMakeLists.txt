cmake_minimum_required(VERSION 3.17)
project(comimant-server C)

set(CMAKE_C_STANDARD 99)

find_package(OpenSSL REQUIRED)

file(GLOB_RECURSE INCLUDE "include/*.h" "include/*.c")

file(GLOB_RECURSE Server_SOURCES "src/*.c")
file(GLOB_RECURSE Server_HEADERS "src/*.h")

file(GLOB_RECURSE Client_SOURCES "client/*.c")
file(GLOB_RECURSE Client_HEADERS "client/*.h")

add_executable(comimant-server ${INCLUDE} ${Server_HEADERS} ${Server_SOURCES})
add_executable(comimant-client ${INCLUDE} ${Client_HEADERS} ${Client_SOURCES})

target_link_libraries(comimant-server OpenSSL::SSL OpenSSL::Crypto)
