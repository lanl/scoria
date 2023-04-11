#ifndef CONTROLLER_HANDLE_REQUESTS
#define CONTROLLER_HANDLE_REQUESTS

#include "controller.h"
#include "request.h"

struct scoria_controller_handler_args {
  int id;
  struct controller *controller;
};

void scoria_controller_status(c_status stat, struct request *req);

c_status scoria_controller_handle_read(struct controller *controller,
                                       struct request_queue *queue,
                                       struct request *req);
c_status scoria_controller_handle_write(struct controller *controller,
                                        struct request_queue *queue,
                                        struct request *req);
c_status scoria_controller_handle_writeadd(struct controller *controller,
                                           struct request_queue *queue,
                                           struct request *req);

void *scoria_controller_req_handler(void *args);
void scoria_controller_handle_requests(struct controller *controller);

#endif /* CONTROLLER_HANDLE_REQUESTS */
