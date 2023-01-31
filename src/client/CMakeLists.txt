cmake_minimum_required(VERSION 3.12)
project(scoria C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -pedantic -Werror)

set(CLIENT_SOURCE_FILES ${CLIENT_SOURCE_FILES} 
                        ${CMAKE_CURRENT_SOURCE_DIR}/client_cleanup.c 
                        ${CMAKE_CURRENT_SOURCE_DIR}/client_init.c 
                        ${CMAKE_CURRENT_SOURCE_DIR}/client_memory.c 
                        ${CMAKE_CURRENT_SOURCE_DIR}/client_wait_requests.c 
                        ${CMAKE_CURRENT_SOURCE_DIR}/client_read_location.c 
                        PARENT_SCOPE)
