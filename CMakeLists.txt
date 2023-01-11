cmake_minimum_required(VERSION 3.10)
project(memory-accelerator C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)

add_compile_options(-Wall -Wextra -pedantic -Wno-unused-function -Wno-strict-prototypes -g)

# JL: Can't use STREQUAL here because the compiler ID could be "IntelLLVM"
if (CMAKE_C_COMPILER_ID MATCHES "Intel")
  add_compile_options(-mavx512f -Wno-debug-disables-optimization)
else()
  add_compile_options(-g)
endif()

set(CMAKE_C_STANDARD 11)

include_directories(include)
include_directories(include/controller)
include_directories(include/client)
include_directories(include/posix)
include_directories(include/shared)
include_directories(include/uthash)

add_executable(controller controller.c ${CONTROLLER_SOURCE_FILES} ${SHARED_SOURCE_FILES})
target_link_libraries(controller shm pthread rt)

add_subdirectory(clients)
add_subdirectory(tests)
