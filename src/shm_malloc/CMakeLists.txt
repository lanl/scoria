cmake_minimum_required(VERSION 3.10)
project(shm_malloc C)

add_library(shm STATIC malloc.c malloc.h)
target_compile_definitions(shm PUBLIC SHM)
target_include_directories(shm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
