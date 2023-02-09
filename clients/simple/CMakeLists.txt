cmake_minimum_required(VERSION 3.12)
project(scoria C)

add_executable(simple_client simple_client.c ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_link_libraries(simple_client shm pthread)
else()
  target_link_libraries(simple_client shm pthread rt)
endif()
