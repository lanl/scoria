#include "request.h"

#include "shm_malloc.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void request_queue_init(request_queue *rq) {
  rq->client = -1;
  rq->active = 0;

  rq->begin = &(rq->requests[0]);
  rq->end = &(rq->requests[REQUEST_QUEUE_SIZE - 1]);

  rq->head = rq->begin;
  rq->tail = rq->begin;

  rq->count = 0;
  rq->size = sizeof(struct request);

  rq->capacity = REQUEST_QUEUE_SIZE;

  pthread_mutexattr_init(&(rq->attr_lock));
  pthread_mutexattr_setpshared(&(rq->attr_lock), PTHREAD_PROCESS_SHARED);

  pthread_mutex_init(&(rq->lock), &(rq->attr_lock));

  pthread_condattr_init(&(rq->attr_empty));
  pthread_condattr_setpshared(&(rq->attr_empty), PTHREAD_PROCESS_SHARED);

  pthread_cond_init(&(rq->empty), &(rq->attr_empty));

  pthread_condattr_init(&(rq->attr_fill));
  pthread_condattr_setpshared(&(rq->attr_fill), PTHREAD_PROCESS_SHARED);

  pthread_cond_init(&(rq->fill), &(rq->attr_fill));
}

void request_queue_free(request_queue *rq) {
  pthread_mutex_destroy(&(rq->lock));
  pthread_mutexattr_destroy(&(rq->attr_lock));

  pthread_cond_destroy(&(rq->empty));
  pthread_condattr_destroy(&(rq->attr_empty));

  pthread_cond_destroy(&(rq->fill));
  pthread_condattr_destroy(&(rq->attr_fill));
}

void request_queue_put(request_queue *rq, const struct request *item) {
  pthread_mutex_lock(&(rq->lock));

  while (rq->count == rq->capacity)
    pthread_cond_wait(&(rq->empty), &(rq->lock));

  memcpy(rq->head, item, rq->size);

  rq->head = rq->head + 1;

  if (rq->head == rq->end)
    rq->head = rq->begin;

  rq->count++;

  pthread_cond_signal(&(rq->fill));
  pthread_mutex_unlock(&(rq->lock));
}

void request_queue_fetch(request_queue *rq, struct request *item) {
  pthread_mutex_lock(&(rq->lock));

  while (rq->count == 0)
    pthread_cond_wait(&(rq->fill), &(rq->lock));

  memcpy(item, rq->tail, rq->size);

  rq->tail = rq->tail + 1;

  if (rq->tail == rq->end)
    rq->tail = rq->begin;

  rq->count--;

  pthread_cond_signal(&(rq->empty));
  pthread_mutex_unlock(&(rq->lock));
}

void request_queue_activate(request_queue *rq, int id) {
  pthread_mutex_lock(&(rq->lock));

  assert(rq->client == -1);
  assert(rq->active == 0);

  rq->client = id;
  rq->active = 1;

  pthread_mutex_unlock(&(rq->lock));
}

void request_queue_deactivate(request_queue *rq) {
  pthread_mutex_lock(&(rq->lock));

  rq->client = -1;
  rq->active = 0;

  rq->begin = &(rq->requests[0]);
  rq->end = &(rq->requests[REQUEST_QUEUE_SIZE - 1]);

  rq->head = rq->begin;
  rq->tail = rq->begin;

  rq->count = 0;
  rq->size = sizeof(struct request);

  rq->capacity = REQUEST_QUEUE_SIZE;

  pthread_mutex_unlock(&(rq->lock));
}

void request_queue_list_init(request_queue_list *rql) {
  for (size_t i = 0; i < MAX_CLIENTS; ++i)
    request_queue_init(&(rql->queues[i]));
}

void request_queue_list_free(request_queue_list *rql) {
  for (size_t i = 0; i < MAX_CLIENTS; ++i)
    request_queue_free(&(rql->queues[i]));
}
