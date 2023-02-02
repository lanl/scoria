cmake_minimum_required(VERSION 3.12)
project(scoria C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)

add_compile_options(-Wall -Wextra -pedantic -Wno-unused-function -Wno-strict-prototypes -g)

include_directories(include)
include_directories(include/controller)
include_directories(include/client)
include_directories(include/posix)
include_directories(include/shared)
include_directories(include/uthash)

# set supported intrinsics
set(SCORIA_SUPPORTED_INTRINSICS AVX SVE)

foreach(_intrinsic ${SCORIA_SUPPORTED_INTRINSICS})
  option(Scoria_REQUIRE_${_intrinsic} "Build Scoria with ${intrinsic} support" OFF)
endforeach()

if(Scoria_REQUIRE_AVX)
  message(STATUS "Building with AVX support")
  add_compile_options(-mavx512f)
  add_compile_definitions(USE_AVX)
else()
  message(STATUS "Building without AVX suppoort")
endif()

if(Scoria_REQUIRE_SVE)
  message(FATAL_ERROR "Scoria does not have SVE support at this time")
else()
  message(STATUS "Building without SVE support")
endif()

if (CMAKE_C_COMPILER_ID MATCHES "Intel")
  add_compile_options(-Rno-debug-disables-optimization)
endif()


option(Scoria_REQUIRE_CLIENTS "Build Scoria example clients" ON)
if(Scoria_REQUIRE_CLIENTS)
  message(STATUS "Building Example Clients")
  add_subdirectory(clients)
endif()

option(Scoria_REQUIRE_TESTS "Build Scoria with test client" OFF)
if(Scoria_REQUIRE_TESTS)
  message(STATUS "Building with Test Client")
  add_subdirectory(tests)
endif()
add_executable(scoria controller.c ${CONTROLLER_SOURCE_FILES} ${SHARED_SOURCE_FILES})

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_link_libraries(scoria shm pthread)
else()
  target_link_libraries(scoria shm pthread rt)
endif()
