cmake_minimum_required(VERSION 3.12)
project(scoria C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)

add_compile_options(-Wall -Wextra -pedantic -Wno-unused-function -Wno-strict-prototypes -O3)

if(Scoria_REQUIRE_AVX)
  message(STATUS "Building with AVX support")
  add_compile_options(-mavx512f)
else()
  message(STATUS "Building without AVX suppoort")
endif()

if(Scoria_REQUIRE_SVE)
  message(STATUS "Building with SVE support")
  #add_compile_options(-march=armv8.2-a)
else()
  message(STATUS "Building without SVE support")
endif()

include_directories(include)
include_directories(include/controller)
include_directories(include/client)
include_directories(include/posix)
include_directories(include/shared)
include_directories(include/uthash)
include_directories(src/shm_malloc)

add_executable(scoria controller.c ${CONTROLLER_SOURCE_FILES} ${SHARED_SOURCE_FILES})

# set supported intrinsics
set(SCORIA_SUPPORTED_INTRINSICS AVX SVE)

foreach(_intrinsic ${SCORIA_SUPPORTED_INTRINSICS})
  option(Scoria_REQUIRE_${_intrinsic} "Build Scoria with ${intrinsic} support" OFF)
endforeach()


if(Scoria_REQUIRE_AVX)
  target_compile_definitions(scoria PUBLIC USE_AVX)
endif()

if(Scoria_REQUIRE_SVE)
  target_compile_definitions(scoria PUBLIC USE_SVE)
endif()


if (CMAKE_C_COMPILER_ID MATCHES "Intel")
  add_compile_options(-Rno-debug-disables-optimization)
endif()


if(NOT MAX_CLIENTS)
  set(MAX_CLIENTS 1)
endif()
add_definitions(-DMAX_CLIENTS=${MAX_CLIENTS})
message(STATUS "Setting MAX_CLIENTS to ${MAX_CLIENTS}")

if(NOT REQUEST_QUEUE_SIZE)
  set(REQUEST_QUEUE_SIZE 100)
endif()
add_definitions(-DREQUEST_QUEUE_SIZE=${REQUEST_QUEUE_SIZE})
message(STATUS "Setting REQUEST_QUEUE_SIZE to ${REQUEST_QUEUE_SIZE}")


option(Scoria_REQUIRE_TIMING "Build Scoria with timing" OFF)
if (Scoria_REQUIRE_TIMING)
  message(STATUS "Building with Timing Enabled")
  add_compile_definitions(Scoria_REQUIRE_TIMING)
endif()

option(Scoria_REQUIRE_CLIENTS "Build Scoria example clients" ON)
if(Scoria_REQUIRE_CLIENTS)
  message(STATUS "Building Example Clients")
  add_subdirectory(clients)
endif()

option(Scoria_REQUIRE_TESTS "Build Scoria with test client" ON)
option(Scoria_REQUIRE_CALIBRATION_TESTS "Build Scoria with calibration tests" OFF)
option(Scoria_SCALE_BW "Build tests with scaled bandwidth measurements" OFF)
option(Scoria_SINGLE_ALLOC "Build tests with single allocation" OFF)
if(Scoria_REQUIRE_TESTS OR Scoria_REQUIRE_CALIBRATION_TESTS)
  if(Scoria_REQUIRE_TESTS)
    message(STATUS "Building with Test Clients")
  endif()

  if(Scoria_REQUIRE_CALIBRATION_TESTS)
    message(STATUS "Build with Calibrations Tests")
  endif()  

  if(Scoria_SCALE_BW)
    message(STATUS "Building Test Clients with Scaled Bandwidth Measurements")
  endif()

  if(Scoria_SINGLE_ALLOC)
    message(STATUS "Building Test Clients with Single Allocation Policy")
  endif()
  add_subdirectory(tests)
endif()

if(Scoria_SCALE_BW)
  target_compile_definitions(scoria PUBLIC SCALE_BW)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_link_libraries(scoria shm pthread)
else()
  target_link_libraries(scoria shm pthread rt)
endif()
