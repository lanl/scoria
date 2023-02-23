cmake_minimum_required(VERSION 3.12)
project(scoria C)

add_executable(simple_client simple_client.c ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})

if(Scoria_REQUIRE_AVX)
  target_compile_definitions(simple_client PUBLIC USE_AVX)
endif()

if(Scoria_REQUIRE_SVE)
  target_compile_definitions(simple_client PUBLIC USE_SVE)
endif()


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_link_libraries(simple_client shm pthread)
else()
  target_link_libraries(simple_client shm pthread rt)
endif()
