add_executable(simple_client simple_client.c ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})
target_link_libraries(simple_client shm pthread rt)
