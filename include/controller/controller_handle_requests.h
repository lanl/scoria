#ifndef CONTROLLER_HANDLE_REQUESTS
#define CONTROLLER_HANDLE_REQUESTS

#include "controller.h"
#include "request.h"

struct thread_args {
  size_t i;
  struct controller *controller;
};

void handle_read(struct controller *controller, struct request_queue *queue,
                 struct request *req);
void handle_write(struct controller *controller, struct request_queue *queue,
                  struct request *req);

void *handler(void *args);
void handle_requests(struct controller *controller);

#endif /* CONTROLLER_HANDLE_REQUESTS */
