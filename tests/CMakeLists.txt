add_executable(test test.c ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})

add_executable(test_client test.c ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})

if(Scoria_REQUIRE_AVX)
  target_compile_definitions(test_client PUBLIC USE_AVX)
endif()
if(Scoria_REQUIRE_SVE)
  target_compile_definitions(test_client PUBLIC USE_SVE)
endif()

target_compile_definitions(test_client PUBLIC USE_CLIENT)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_link_libraries(test shm pthread)
  target_link_libraries(test_client shm pthread)
else()
  target_link_libraries(test shm pthread rt)
  target_link_libraries(test_client shm pthread rt)
endif()
