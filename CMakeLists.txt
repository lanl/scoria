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

add_executable(client client.c ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})
target_link_libraries(client shm pthread rt)

add_executable(controller controller.c ${CONTROLLER_SOURCE_FILES} ${SHARED_SOURCE_FILES})
target_link_libraries(controller shm pthread rt)


find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/argtable3/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

add_subdirectory(external/argtable3)
include_directories(external/argtable3/src)

add_subdirectory(tests)
