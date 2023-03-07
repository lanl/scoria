#include "controller_handle_requests.h"

#include "config.h"
#include "controller.h"
#include "kernels.h"
#include "mytimer.h"
#include "request.h"
#include "utils.h"

#include "shm_malloc.h"

#include <stdio.h>

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
    printf("Controller: Client(%d): Error: Unrecognized intrinsic type\n",
           req->client);
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

  uint64_t mtime = 0;
#ifdef Scoria_REQUIRE_TIMING
  double bw_mult = (double)(req->N * sizeof(double));
  bw_mult *= 1e9 / (1024.0 * 1024.0 * 1024.0);
#endif /* Scoria_REQUIRE_TIMING */

  c_status stat = SCORIA_SUCCESS;

  if (req->ind1 == NULL) {
    assert(req->ind2 == NULL);
    if (req->nthreads == 0) {
      TIME(
          {
            stat = read_single_thread_0(req->output, req->input, req->N,
                                        req->intrinsics);
          },
          mtime)
    } else {
      TIME(
          {
            stat = read_multi_thread_0(req->output, req->input, req->N,
                                       req->nthreads, req->intrinsics);
          },
          mtime)
    }

#ifdef Scoria_REQUIRE_TIMING
    double bw = bw_mult / (double)mtime;
    req->nsecs = mtime;
#endif

    req->r_status = Ready;

    request_queue_put(queue, req);

    if (stat != SCORIA_SUCCESS) {
      controller_status(stat, req);
    } else {
      if (controller->chatty) {
#ifdef Scoria_REQUIRE_TIMING
#ifdef SCALE_BW
        double factor = 2.0;
#else
        double factor = 2.0;
#endif /* SCALE_BW */
        printf("Controller: Client(%d) Read Data with N: %ld Time (ns): %ld "
               "Bandwidth: %f GiB/s\n",
               req->client, req->N, mtime, factor * bw);
#else
        printf("Controller: Client(%d) Read Data with N: %ld\n", req->client,
               req->N);
#endif /* Scoria_REQUIRE_TIMING */
      }
    }
    return stat;
  }

  if (req->ind2 == NULL) {
    assert(req->ind1 != NULL);
    if (req->nthreads == 0) {
      TIME(
          {
            stat = read_single_thread_1(req->output, req->input, req->N,
                                        req->ind1, req->intrinsics);
          },
          mtime)
    } else {
      TIME(
          {
            stat =
                read_multi_thread_1(req->output, req->input, req->N, req->ind1,
                                    req->nthreads, req->intrinsics);
          },
          mtime)
    }

#ifdef Scoria_REQUIRE_TIMING
    double bw = bw_mult / (double)mtime;
    req->nsecs = mtime;
#endif /* Scoria_REQUIRE_TIMING */

    req->r_status = Ready;

    request_queue_put(queue, req);

    if (stat != SCORIA_SUCCESS) {
      controller_status(stat, req);
    } else {
      if (controller->chatty) {
#ifdef Scoria_REQUIRE_TIMING
#ifdef SCALE_BW
        double factor = 3.0;
#else
        double factor = 2.0;
#endif /* SCALE_BW */
        printf("Controller: Client(%d) Read Data with N: %ld Time (ns): %ld "
               "Bandwidth: %f GiB/s\n",
               req->client, req->N, mtime, factor * bw);
#else
        printf("Controller: Client(%d) Read Data with N: %ld\n", req->client,
               req->N);
#endif /* Scoria_REQUIRE_TIMING */
      }
    }
    return stat;
  }

  assert(req->ind1 != NULL);
  assert(req->ind2 != NULL);

  if (req->nthreads == 0) {
    TIME(
        {
          stat = read_single_thread_2(req->output, req->input, req->N,
                                      req->ind1, req->ind2, req->intrinsics);
        },
        mtime)
  } else {
    TIME(
        {
          stat = read_multi_thread_2(req->output, req->input, req->N, req->ind1,
                                     req->ind2, req->nthreads, req->intrinsics);
        },
        mtime)
  }

#ifdef Scoria_REQUIRE_TIMING
  double bw = bw_mult / (double)mtime;
  req->nsecs = mtime;
#endif /* Scoria_REQUIRE_TIMING */

  req->r_status = Ready;

  request_queue_put(queue, req);

  if (stat != SCORIA_SUCCESS) {
    controller_status(stat, req);
  } else {
    if (controller->chatty) {
#ifdef Scoria_REQUIRE_TIMING
#ifdef SCALE_BW
      double factor = 2.0;
#else
      double factor = 4.0;
#endif /* SCALE_BW */
      printf("Controller: Client(%d) Read Data with N: %ld Time (ns): %ld "
             "Bandwidth: %f GiB/s\n",
             req->client, req->N, mtime, factor * bw);
#else
      printf("Controller: Client(%d) Read Data with N: %ld\n", req->client,
             req->N);
#endif /* Scoria_REQUIRE_TIMING */
    }
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

  uint64_t mtime = 0;
#ifdef Scoria_REQUIRE_TIMING
  double bw_mult = (double)(req->N * sizeof(double));
  bw_mult *= 1e9 / (1024.0 * 1024.0 * 1024.0);
#endif /* Scoria_REQUIRE_TIMING */

  c_status stat = SCORIA_SUCCESS;

  if (req->ind1 == NULL) {
    assert(req->ind2 == NULL);
    if (req->nthreads == 0) {
      TIME(
          {
            stat = write_single_thread_0(req->output, req->input, req->N,
                                         req->intrinsics);
          },
          mtime)
    } else {
      TIME(
          {
            stat = write_multi_thread_0(req->output, req->input, req->N,
                                        req->nthreads, req->intrinsics);
          },
          mtime)
    }

#ifdef Scoria_REQUIRE_TIMING
    double bw = bw_mult / (double)mtime;
    req->nsecs = mtime;
#endif /* Scoria_REQUIRE_TIMING */

    req->r_status = Ready;

    request_queue_put(queue, req);

    if (stat != SCORIA_SUCCESS) {
      controller_status(stat, req);
    } else {
      if (controller->chatty) {
#ifdef Scoria_REQUIRE_TIMING
#ifdef SCALE_BW
        double factor = 2.0;
#else
        double factor = 2.0;
#endif /* SCALE_BW */
        printf("Controller: Client(%d) Write Data with N: %ld Time (ns): %ld "
               "Bandwidth: %f GiB/s\n",
               req->client, req->N, mtime, factor * bw);
#else
        printf("Controller: Client(%d) Write Data with N: %ld\n", req->client,
               req->N);
#endif /* Scoria_REQUIRE_TIMING */
      }
    }

    return stat;
  }

  if (req->ind2 == NULL) {
    assert(req->ind1 != NULL);
    if (req->nthreads == 0) {
      TIME(
          {
            stat = write_single_thread_1(req->output, req->input, req->N,
                                         req->ind1, req->intrinsics);
          },
          mtime)
    } else {
      TIME(
          {
            stat =
                write_multi_thread_1(req->output, req->input, req->N, req->ind1,
                                     req->nthreads, req->intrinsics);
          },
          mtime)
    }

#ifdef Scoria_REQUIRE_TIMING
    double bw = bw_mult / (double)mtime;
    req->nsecs = mtime;
#endif /* Scoria_REQUIRE_TIMING */

    req->r_status = Ready;

    request_queue_put(queue, req);

    if (stat != SCORIA_SUCCESS) {
      controller_status(stat, req);
    } else {
      if (controller->chatty) {
#ifdef Scoria_REQUIRE_TIMING
#ifdef SCALE_BW
        double factor = 2.0;
#else
        double factor = 3.0;
#endif /* SCALE_BW */
        printf("Controller: Client(%d) Write Data with N: %ld Time (ns): %ld "
               "Bandwidth: %f GiB/s\n",
               req->client, req->N, mtime, factor * bw);
#else
        printf("Controller: Client(%d) Write Data with N: %ld\n", req->client,
               req->N);
#endif /* Scoria_REQUIRE_TIMING */
      }
    }

    return stat;
  }

  assert(req->ind1 != NULL);
  assert(req->ind2 != NULL);

  if (req->nthreads == 0) {
    TIME(
        {
          stat = write_single_thread_2(req->output, req->input, req->N,
                                       req->ind1, req->ind2, req->intrinsics);
        },
        mtime)
  } else {
    TIME(
        {
          stat =
              write_multi_thread_2(req->output, req->input, req->N, req->ind1,
                                   req->ind2, req->nthreads, req->intrinsics);
        },
        mtime)
  }

#ifdef Scoria_REQUIRE_TIMING
  double bw = bw_mult / (double)mtime;
  req->nsecs = mtime;
#endif /* Scoria_REQUIRE_TIMING */

  req->r_status = Ready;

  request_queue_put(queue, req);

  if (stat != SCORIA_SUCCESS) {
    controller_status(stat, req);
  } else {
    if (controller->chatty) {
#ifdef Scoria_REQUIRE_TIMING
#ifdef SCALE_BW
      double factor = 2.0;
#else
      double factor = 4.0;
#endif /* SCALE_BW */
      printf("Controller: Client(%d) Write Data with N: %ld Time (ns): %ld "
             "Bandwidth: %f GiB/s\n",
             req->client, req->N, mtime, factor * bw);
#else
      printf("Controller: Client(%d) Write Data with N: %ld\n", req->client,
             req->N);
#endif /* Scoria_REQUIRE_TIMING */
    }
  }

  return stat;
}

c_status handle_writeadd(struct controller *controller,
                         struct request_queue *queue, struct request *req) {
  if (controller->chatty)
    printf("Controller: Received Request Object: Client: %d ID: %d Type: %d "
           "Pointer: %p Input Pointer: %p N: %ld\n",
           req->client, req->id, req->r_type, (void *)req->output,
           (void *)req->input, req->N);

  uint64_t mtime = 0;
#ifdef Scoria_REQUIRE_TIMING
  double bw_mult = (double)(req->N * sizeof(double));
  bw_mult *= 1e9 / (1024.0 * 1024.0 * 1024.0);
#endif /* Scoria_REQUIRE_TIMING */

  c_status stat = SCORIA_SUCCESS;

  if (req->ind1 == NULL) {
    assert(req->ind2 == NULL);
    if (req->nthreads == 0) {
      TIME(
          {
            stat = writeadd_single_thread_0(req->output, req->input, req->N,
                                            req->intrinsics);
          },
          mtime)
    } else {
      TIME(
          {
            stat = writeadd_multi_thread_0(req->output, req->input, req->N,
                                           req->nthreads, req->intrinsics);
          },
          mtime)
    }

#ifdef Scoria_REQUIRE_TIMING
    double bw = bw_mult / (double)mtime;
    req->nsecs = mtime;
#endif /* Scoria_REQUIRE_TIMING */

    req->r_status = Ready;

    request_queue_put(queue, req);

    if (stat != SCORIA_SUCCESS) {
      controller_status(stat, req);
    } else {
      if (controller->chatty) {
#ifdef Scoria_REQUIRE_TIMING
#ifdef SCALE_BW
        double factor = 2.0;
#else
        double factor = 2.0;
#endif /* SCALE_BW */
        printf("Controller: Client(%d) Write Data with N: %ld Time (ns): %ld "
               "Bandwidth: %f GiB/s\n",
               req->client, req->N, mtime, factor * bw);
#else
        printf("Controller: Client(%d) Write Data with N: %ld\n", req->client,
               req->N);
#endif /* Scoria_REQUIRE_TIMING */
      }
    }

    return stat;
  }

  if (req->ind2 == NULL) {
    assert(req->ind1 != NULL);
    if (req->nthreads == 0) {
      TIME(
          {
            stat = writeadd_single_thread_1(req->output, req->input, req->N,
                                            req->ind1, req->intrinsics);
          },
          mtime)
    } else {
      TIME(
          {
            stat = writeadd_multi_thread_1(req->output, req->input, req->N,
                                           req->ind1, req->nthreads,
                                           req->intrinsics);
          },
          mtime)
    }

#ifdef Scoria_REQUIRE_TIMING
    double bw = bw_mult / (double)mtime;
    req->nsecs = mtime;
#endif /* Scoria_REQUIRE_TIMING */

    req->r_status = Ready;

    request_queue_put(queue, req);

    if (stat != SCORIA_SUCCESS) {
      controller_status(stat, req);
    } else {
      if (controller->chatty) {
#ifdef Scoria_REQUIRE_TIMING
#ifdef SCALE_BW
        double factor = 2.0;
#else
        double factor = 3.0;
#endif /* SCALE_BW */
        printf("Controller: Client(%d) Write Data with N: %ld Time (ns): %ld "
               "Bandwidth: %f GiB/s\n",
               req->client, req->N, mtime, factor * bw);
#else
        printf("Controller: Client(%d) Write Data with N: %ld\n", req->client,
               req->N);
#endif /* Scoria_REQUIRE_TIMING */
      }
    }

    return stat;
  }

  assert(req->ind1 != NULL);
  assert(req->ind2 != NULL);

  if (req->nthreads == 0) {
    TIME(
        {
          stat =
              writeadd_single_thread_2(req->output, req->input, req->N,
                                       req->ind1, req->ind2, req->intrinsics);
        },
        mtime)
  } else {
    TIME(
        {
          stat = writeadd_multi_thread_2(req->output, req->input, req->N,
                                         req->ind1, req->ind2, req->nthreads,
                                         req->intrinsics);
        },
        mtime)
  }

#ifdef Scoria_REQUIRE_TIMING
  double bw = bw_mult / (double)mtime;
  req->nsecs = mtime;
#endif /* Scoria_REQUIRE_TIMING */

  req->r_status = Ready;

  request_queue_put(queue, req);

  if (stat != SCORIA_SUCCESS) {
    controller_status(stat, req);
  } else {
    if (controller->chatty) {
#ifdef Scoria_REQUIRE_TIMING
#ifdef SCALE_BW
      double factor = 2.0;
#else
      double factor = 4.0;
#endif /* SCALE_BW */
      printf("Controller: Client(%d) Write Data with N: %ld Time (ns): %ld "
             "Bandwidth: %f GiB/s\n",
             req->client, req->N, mtime, factor * bw);
#else
      printf("Controller: Client(%d) Write Data with N: %ld\n", req->client,
             req->N);
#endif /* Scoria_REQUIRE_TIMING */
    }
  }

  return stat;
}

void *handler(void *args) {
  struct thread_args *a = args;

  int quit = 0;
  int tid = a->id;
  struct controller *controller = a->controller;

  struct request_queue *requests =
      &(controller->shared_requests_list->queues[tid]);
  struct request_queue *completions =
      &(controller->shared_completions_list->queues[tid]);

  c_status stat = SCORIA_SUCCESS;
  while (!quit) {
    struct request req;
    request_queue_fetch(requests, &req);

    if (controller->chatty)
      printf("Controller: Client (%d): Request %d Detected of Type %d\n", tid,
             req.id, req.r_type);

    switch (req.r_type) {
    case Read:
      stat = handle_read(controller, completions, &req);
      if (stat != SCORIA_SUCCESS)
        quit = 1;
      break;
    case Write:
      stat = handle_write(controller, completions, &req);
      if (stat != SCORIA_SUCCESS)
        quit = 1;
      break;
    case WriteAdd:
      stat = handle_writeadd(controller, completions, &req);
      if (stat != SCORIA_SUCCESS)
        quit = 1;
      break;
    case Quit:
      quit = 1;
      req.r_status = Ready;
      request_queue_put(completions, &req);

      for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (i != tid) {
          struct request req;
          req.r_type = Kill;
          req.id = -1;
          req.client = tid;
          request_queue_put(&(controller->shared_completions_list->queues[i]),
                            &req);
          request_queue_put(&(controller->shared_requests_list->queues[i]),
                            &req);
        }
      }
      break;
    case Kill:
      quit = 1;
      break;
    default:
      printf("Controller: Client (%d): Invalid Request Type Detected\n", tid);
      quit = 1;
    }
  }

  if (stat != SCORIA_SUCCESS) {
    printf("Controller: Client (%d): Error detected\n", tid);
    printf("Quitting...\n");
    quit = 1;
  }
  return NULL;
}

void handle_requests(struct controller *controller) {
  // Start loop
  pthread_t threads[MAX_CLIENTS];
  struct thread_args args[MAX_CLIENTS];

  for (int i = 0; i < MAX_CLIENTS; ++i) {
    args[i].id = i;
    args[i].controller = controller;

    int ret = pthread_create(&threads[i], NULL, handler, &args[i]);
    (void)ret;
    assert(ret == 0);
  }

  for (int i = 0; i < MAX_CLIENTS; ++i) {
    pthread_join(threads[i], NULL);
  }

  printf("Controller: Quit Request Received from Client\n");
}
