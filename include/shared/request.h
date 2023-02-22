#ifndef REQUEST_H
#define REQUEST_H

#include "uthash.h"

#include "config.h"

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum { Read, Write, Quit, Kill } request_type;
typedef enum { Waiting, Ready } request_status;

struct request {
  int client;
  int id;

  request_type r_type;
  request_status r_status;
  size_t size;

  size_t N;

  const void *input;
  void *output;

  const size_t *ind1;
  const size_t *ind2;
  size_t nthreads;
  bool use_avx;

  size_t offset;
  double value;

  uint64_t nsecs;

  UT_hash_handle hh;
};

typedef struct request_queue {
  int client;
  int active;

  struct request requests[REQUEST_QUEUE_SIZE];

  struct request *head;
  struct request *tail;

  size_t capacity;
  size_t count;
  size_t size;

  struct request *begin;
  struct request *end;

  pthread_mutexattr_t attr_lock;
  pthread_condattr_t attr_empty;
  pthread_condattr_t attr_fill;

  pthread_mutex_t lock;
  pthread_cond_t empty, fill;
} request_queue;

void request_queue_init(request_queue *rq);
void request_queue_free(request_queue *rq);

void request_queue_put(request_queue *rq, const struct request *item);
void request_queue_fetch(request_queue *rq, struct request *item);

void request_queue_activate(request_queue *rq, int id);
void request_queue_deactivate(request_queue *rq);

typedef struct request_queue_list {
  struct request_queue queues[MAX_CLIENTS];
} request_queue_list;

void request_queue_list_init(request_queue_list *rql);
void request_queue_list_free(request_queue_list *rql);

#endif /* REQUEST_H */
