add_executable(test test.c ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})
target_link_libraries(test shm pthread rt)

add_executable(test_client test.c ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})
target_compile_definitions(test_client PUBLIC USE_CLIENT)
target_link_libraries(test_client shm pthread rt)

add_executable(test_spatter test_spatter.c parse-args.c json.c sp_alloc.c pcg_basic.c backend-support-tests.c ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})
target_compile_definitions(test_spatter PUBLIC USE_SERIAL)
target_link_libraries(test_spatter shm pthread rt argtable3)
