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

set(CMAKE_C_VISIBILITY_PRESET hidden)

add_library(comimant-server-api SHARED ${INCLUDE} ${Server_SOURCES} ${Server_HEADERS})
set_target_properties(comimant-server-api PROPERTIES PREFIX "" DEFINE_SYMBOL "")
set_target_properties(comimant-server-api PROPERTIES OUTPUT_NAME "comimant")

target_link_libraries(comimant-server OpenSSL::SSL OpenSSL::Crypto ${CMAKE_DL_LIBS})
