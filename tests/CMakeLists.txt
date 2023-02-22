add_executable(test test.c ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})

add_executable(test_client test.c ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})

add_executable(test_calibrate calibrate.c ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})

if(Scoria_REQUIRE_AVX)
  target_compile_definitions(test_client PUBLIC USE_AVX)
endif()
if(Scoria_REQUIRE_SVE)
  message(FATAL_ERROR "Scoria does not have SVE support at this time")
endif()

target_compile_definitions(test_client PUBLIC USE_CLIENT)
target_compile_definitions(test_calibrate PUBLIC USE_CLIENT)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_link_libraries(test shm pthread numa)
  target_link_libraries(test_client shm pthread numa)
  target_link_libraries(test_calibrate shm pthread numa)
else()
  target_link_libraries(test shm pthread numa rt)
  target_link_libraries(test_client shm pthread numa rt)
  target_link_libraries(test_calibrate shm pthread numa rt)
endif()
