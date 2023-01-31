cmake_minimum_required(VERSION 3.12)
project(scooria C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -pedantic -Werror)

set(CONTROLLER_SOURCE_FILES ${CONTROLLER_SOURCE_FILES} 
                            ${CMAKE_CURRENT_SOURCE_DIR}/controller_cleanup.c  
                            ${CMAKE_CURRENT_SOURCE_DIR}/controller_handle_requests.c  
                            ${CMAKE_CURRENT_SOURCE_DIR}/controller_init.c  
                            ${CMAKE_CURRENT_SOURCE_DIR}/controller_write_location.c
                            PARENT_SCOPE)
