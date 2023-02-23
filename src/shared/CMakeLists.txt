cmake_minimum_required(VERSION 3.12)
project(scoria C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -pedantic -Werror)

set(SHARED_SOURCE_FILES ${SHARED_SOURCE_FILES} 
                        ${CMAKE_CURRENT_SOURCE_DIR}/request.c 
                        ${CMAKE_CURRENT_SOURCE_DIR}/utils.c
                        ${CMAKE_CURRENT_SOURCE_DIR}/mytimer.c
                        PARENT_SCOPE)
