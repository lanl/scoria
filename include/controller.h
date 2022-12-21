#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <semaphore.h>

#include "config.h"
#include "request.h"

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
