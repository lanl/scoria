#ifndef CLIENT_H
#define CLIENT_H

#include <semaphore.h>

#include "config.h"
#include "request.h"

struct client {
  int id;

  int fd_location;
  int fd_requests;
  int fd_completions;

  int chatty;

  struct memory_location *shared_location;

  struct request_queue_list *shared_requests_list;
  struct request_queue_list *shared_completions_list;
  struct shared_memory *shared_mem_ptr;

  struct request_queue *shared_requests;
  struct request_queue *shared_completions;

  struct request *unmatched_requests;
};

#endif /* CLIENT_H */
