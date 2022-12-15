cmake_minimum_required(VERSION 3.10)
project(memory-accelerator C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(shm_malloc)
add_subdirectory(posix)
add_subdirectory(controller)
add_subdirectory(client)
add_subdirectory(shared)

set(SHARED_SOURCE_FILES ${SHARED_SOURCE_FILES} PARENT_SCOPE)
set(CLIENT_SOURCE_FILES ${CLIENT_SOURCE_FILES} PARENT_SCOPE)
set(CONTROLLER_SOURCE_FILES ${CONTROLLER_SOURCE_FILES} PARENT_SCOPE)

add_compile_options(-Wall -Wextra -pedantic -Werror)
