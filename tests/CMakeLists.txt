if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(TEST_LIBS shm pthread)
else()
  set(TEST_LIBS shm pthread rt)
endif()

if(Scoria_REQUIRE_TESTS)
  add_executable(test test.c ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})
  add_executable(test_client test.c ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})

  if(Scoria_REQUIRE_AVX)
    target_compile_definitions(test_client PUBLIC USE_AVX)
  endif()

  if(Scoria_REQUIRE_SVE)
    target_compile_definitions(test_client PUBLIC USE_SVE)
  endif()

  if(Scoria_SCALE_BW)
    target_compile_definitions(test PUBLIC SCALE_BW)
    target_compile_definitions(test_client PUBLIC SCALE_BW)
  endif()

  if(Scoria_SINGLE_ALLOC)
    target_compile_definitions(test_client PUBLIC SINGLE_ALLOC)
  endif()

  target_compile_definitions(test_client PUBLIC USE_CLIENT)

  target_link_libraries(test ${TEST_LIBS})
  target_link_libraries(test_client ${TEST_LIBS})
endif()

if(Scoria_REQUIRE_CALIBRATION_TESTS)
  add_executable(test_calibrate calibrate.c ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})
  add_executable(test_calibrate_client calibrate.c ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})

  target_compile_definitions(test_calibrate_client PUBLIC USE_CLIENT)

  if(Scoria_REQUIRE_AVX)
    target_compile_definitions(test_calibrate_client PUBLIC USE_AVX)
  endif()

  if(Scoria_REQUIRE_SVE)
    target_compile_definitions(test_calibrate_client PUBLIC USE_SVE)
  endif()

  if(Scoria_SCALE_BW)
    target_compile_definitions(test_calibrate PUBLIC SCALE_BW)
    target_compile_definitions(test_calibrate_client PUBLIC SCALE_BW)
  endif()

  if(Scoria_SINGLE_ALLOC)
    target_compile_definitions(test_calibrate_client PUBLIC SINGLE_ALLOC)
  endif()
 
  find_package(OpenMP)
  if(OpenMP_C_FOUND)
    target_link_libraries(test_calibrate OpenMP::OpenMP_C)
    target_link_libraries(test_calibrate_client OpenMP::OpenMP_C)
  endif()

#  find_package(OpenMP)
#  if (OPENMP_FOUND)
#    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
#  endif()

  target_link_libraries(test_calibrate ${TEST_LIBS})
  target_link_libraries(test_calibrate_client ${TEST_LIBS} numa)
endif()
