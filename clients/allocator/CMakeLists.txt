cmake_minimum_required(VERSION 3.12)
project(simple_allocator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

add_executable(simple_allocator simple_allocator.cc shm_allocator.hh ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})

if(Scoria_REQUIRE_AVX)
  target_compile_definitions(simple_allocator PUBLIC USE_AVX)
endif()

if(Scoria_REQUIRE_SVE)
  target_compile_definitions(simple_allocator PUBLIC USE_SVE)
endif()

set_target_properties(simple_allocator PROPERTIES
  COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
  COMPILE_OPTIONS "${WARNING_FLAGS}"
  )

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_link_libraries(simple_allocator shm pthread)
else()
  target_link_libraries(simple_allocator shm pthread rt)
endif()
