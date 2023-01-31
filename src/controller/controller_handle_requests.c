#include "controller_handle_requests.h"

#include "config.h"
#include "controller.h"
#include "kernels.h"
#include "request.h"
#include "utils.h"

#include "shm_malloc.h"

#include <stdio.h>

int quit = 0;
int tid = -1;

void controller_status(c_status stat, struct request *req) {
  switch (stat) {
  case SCORIA_SUCCESS:
    printf("Controller: Client(%d): Error: No error\n", req->client);
    break;
  case SCORIA_SCALAR_READ_FAIL:
    printf("Controller: Client(%d): Error: Scalar read error\n", req->client);
    break;
  case SCORIA_SCALAR_WRITE_FAIL:
    printf("Controller: Client(%d): Error: Scalar write error\n", req->client);
    break;
  case SCORIA_AVX_READ_FAIL:
    printf("Controller: Client(%d): Error: AVX read error\n", req->client);
    break;
  case SCORIA_AVX_WRITE_FAIL:
    printf("Controller: Client(%d): Error: AVX write error\n", req->client);
    break;
  case SCORIA_SVE_READ_FAIL:
    printf("Controller: Client(%d): Error: SVE read error\n", req->client);
    break;
  case SCORIA_SVE_WRITE_FAIL:
    printf("Controller: Client(%d): Error: SVE write error\n", req->client);
    break;
  case SCORIA_INTRINSIC_EXIST:
    printf("Controller: Client(%d): Error: Unrecognized intrinsic type\n", req->client);
    break;
  default:
    printf("Controller: Client(%d): Error: Unknown status code %d\n",
           req->client, stat);
  }
}

c_status handle_read(struct controller *controller, struct request_queue *queue,
                     struct request *req) {
  if (controller->chatty)
    printf("Controller: Received Request Object: Client: %d ID: %d Type: %d N: "
           "%ld\n",
           req->client, req->id, req->r_type, req->N);

  c_status stat = SCORIA_SUCCESS;

  if (req->ind1 == NULL) {
    assert(req->ind2 == NULL);
    if (req->nthreads == 0) {
      stat =
          read_single_thread_0(req->output, req->input, req->N, req->intrinsics);
    } else {
      stat = read_multi_thread_0(req->output, req->input, req->N, req->nthreads,
                                 req->intrinsics);
    }

    req->r_status = Ready;

    request_queue_put(queue, req);

    if (stat != SCORIA_SUCCESS) {
      controller_status(stat, req);
    } else {
      if (controller->chatty)
        printf("Controller: Client(%d) Read Data with N: %ld\n", req->client,
               req->N);
    }
    return stat;
  }

  if (req->ind2 == NULL) {
    assert(req->ind1 != NULL);
    if (req->nthreads == 0) {
      stat = read_single_thread_1(req->output, req->input, req->N, req->ind1,
                                  req->intrinsics);
    } else {
      stat = read_multi_thread_1(req->output, req->input, req->N, req->ind1,
                                 req->nthreads, req->intrinsics);
    }

    req->r_status = Ready;

    request_queue_put(queue, req);

    if (stat != SCORIA_SUCCESS) {
      controller_status(stat, req);
    } else {
      if (controller->chatty)
        printf("Controller: Client(%d) Read Data with N: %ld\n", req->client,
               req->N);
    }
    return stat;
  }

  assert(req->ind1 != NULL);
  assert(req->ind2 != NULL);

  if (req->nthreads == 0) {
    stat = read_single_thread_2(req->output, req->input, req->N, req->ind1,
                                req->ind2, req->intrinsics);
  } else {
    stat = read_multi_thread_2(req->output, req->input, req->N, req->ind1,
                               req->ind2, req->nthreads, req->intrinsics);
  }

  req->r_status = Ready;

  request_queue_put(queue, req);

  if (stat != SCORIA_SUCCESS) {
    controller_status(stat, req);
  } else {
    if (controller->chatty)
      printf("Controller: Client(%d) Read Data with N: %ld\n", req->client,
             req->N);
  }
  return stat;
}

c_status handle_write(struct controller *controller,
                      struct request_queue *queue, struct request *req) {
  if (controller->chatty)
    printf("Controller: Received Request Object: Client: %d ID: %d Type: %d "
           "Pointer: %p Input Pointer: %p N: %ld\n",
           req->client, req->id, req->r_type, (void *)req->output,
           (void *)req->input, req->N);

  c_status stat = SCORIA_SUCCESS;

  if (req->ind1 == NULL) {
    assert(req->ind2 == NULL);
    if (req->nthreads == 0) {
      stat =
          write_single_thread_0(req->output, req->input, req->N, req->intrinsics);
    } else {
      stat = write_multi_thread_0(req->output, req->input, req->N,
                                  req->nthreads, req->intrinsics);
    }

    req->r_status = Ready;

    request_queue_put(queue, req);

    if (stat != SCORIA_SUCCESS) {
      controller_status(stat, req);
    } else {
      if (controller->chatty)
        printf("Controller: Client(%d) Write Data with N: %ld\n", req->client,
               req->N);
    }

    return stat;
  }

  if (req->ind2 == NULL) {
    assert(req->ind1 != NULL);
    if (req->nthreads == 0) {
      stat = write_single_thread_1(req->output, req->input, req->N, req->ind1,
                                   req->intrinsics);
    } else {
      stat = write_multi_thread_1(req->output, req->input, req->N, req->ind1,
                                  req->nthreads, req->intrinsics);
    }

    req->r_status = Ready;

    request_queue_put(queue, req);

    if (stat != SCORIA_SUCCESS) {
      controller_status(stat, req);
    } else {
      if (controller->chatty)
        printf("Controller: Client(%d) Write Data with N: %ld\n", req->client,
               req->N);
    }

    return stat;
  }

  assert(req->ind1 != NULL);
  assert(req->ind2 != NULL);

  if (req->nthreads == 0) {
    stat = write_single_thread_2(req->output, req->input, req->N, req->ind1,
                                 req->ind2, req->intrinsics);
  } else {
    stat = write_multi_thread_2(req->output, req->input, req->N, req->ind1,
                                req->ind2, req->nthreads, req->intrinsics);
  }

  req->r_status = Ready;

  request_queue_put(queue, req);

  if (stat != SCORIA_SUCCESS) {
    controller_status(stat, req);
  } else {
    if (controller->chatty)
      printf("Controller: Client(%d) Write Data with N: %ld\n", req->client,
             req->N);
  }

  return stat;
}

void *handler(void *args) {
  struct thread_args *a = args;

  size_t i = a->i;
  struct controller *controller = a->controller;

  struct request_queue *requests =
      &(controller->shared_requests_list->queues[i]);
  struct request_queue *completions =
      &(controller->shared_completions_list->queues[i]);

  c_status stat = SCORIA_SUCCESS;
  while (!quit) {
    struct request req;
    request_queue_fetch(requests, &req);

    if (controller->chatty)
      printf("Controller: Client (%ld): Request %d Detected\n", i, req.id);

    switch (req.r_type) {
    case Read:
      stat = handle_read(controller, completions, &req);
      break;
    case Write:
      stat = handle_write(controller, completions, &req);
      break;
    case Quit:
      tid = i;
      quit = 1;
      req.r_status = Ready;
      request_queue_put(completions, &req);
      break;
    case Kill:
      quit = 1;
      break;
    default:
      printf("Controller: Client (%ld): Invalid Request Type Detected\n", i);
      tid = i;
      quit = 1;
    }

    if (stat != SCORIA_SUCCESS) {
      printf("Controller: Error detected\n");
      printf("Quitting...\n");
      quit = 1;
    }
  }

  return NULL;
}

void handle_requests(struct controller *controller) {
  // Start loop
  pthread_t threads[MAX_CLIENTS];
  struct thread_args args[MAX_CLIENTS];

  for (size_t i = 0; i < MAX_CLIENTS; ++i) {
    args[i].i = i;
    args[i].controller = controller;

    int ret = pthread_create(&threads[i], NULL, handler, &args[i]);
    (void)ret;
    assert(ret == 0);
  }

  while (!quit) {
    ;
    ;
  }

  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (i != tid) {
      struct request req;
      req.r_type = Kill;
      req.id = -1;
      req.client = tid;
      request_queue_put(&(controller->shared_completions_list->queues[i]),
                        &req);
      request_queue_put(&(controller->shared_requests_list->queues[i]), &req);
    }
  }

  for (int i = 0; i < MAX_CLIENTS; ++i) {
    pthread_join(threads[i], NULL);
  }

  printf("Controller: Quit Request Received from Client(%d)\n", tid);
}
