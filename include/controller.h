#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <semaphore.h>

#include "config.h"
#include "request.h"

#define NUMERRORS 7

typedef enum _controller_status {
  SCORIA_SUCCESS = 0,
  SCORIA_SCALAR_READ_FAIL = -1,
  SCORIA_SCALAR_WRITE_FAIL = -2,
  SCORIA_AVX_READ_FAIL = -3,
  SCORIA_AVX_WRITE_FAIL = -4,
  SCORIA_SVE_READ_FAIL = -5,
  SCORIA_SVE_WRITE_FAIL = -6
} c_status;

struct controller {
  int fd_location;
  int fd_requests;
  int fd_completions;

  int chatty;

  struct memory_location *shared_location;

  struct request_queue_list *shared_requests_list;
  struct request_queue_list *shared_completions_list;
  struct shared_memory *shared_mem_ptr;
};

#endif /* CONTROLLER_H */
