cmake_minimum_required(VERSION 3.12)
project(scoria C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -pedantic -Werror)

set(SHARED_SOURCE_FILES ${SHARED_SOURCE_FILES} 
                        ${CMAKE_CURRENT_SOURCE_DIR}/posix_sm.c
                        PARENT_SCOPE)
