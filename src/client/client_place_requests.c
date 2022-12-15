#include "client_place_requests.h"
#include "client_memory.h"

#include "client.h"
#include "config.h"
#include "request.h"

#include "shm_malloc.h"
#include "uthash.h"

#include <stdio.h>

void wait_request(struct client *client, struct request *req) {
  if (client->chatty)
    printf("Client(%d): Waiting on Request %d:%d\n", client->id, req->client,
           req->id);

  int found;
  int id = req->id;

  struct request *query;

  HASH_FIND_INT(client->unmatched_requests, &id, query);
  if (query == NULL)
    found = 0;
  else
    found = 1;

  while (!found) {
    struct request complete;
    request_queue_fetch(client->shared_completions, &complete);

    if (complete.r_type == Kill) {
      printf("Received a Kill Request Originating from a Quit Request from "
             "Client(%d)\n",
             complete.client);
      exit(1);
    }

    if (complete.id == id) {
      *req = complete;
      struct request *find;

      HASH_FIND_INT(client->unmatched_requests, &id, find);
      if (find != NULL)
        HASH_DEL(client->unmatched_requests, find);

      found = 1;
    } else
      HASH_ADD_INT(client->unmatched_requests, id, &complete);
  }

  if (client->chatty)
    printf("Client(%d): Controller Completed Request %d:%d\n", client->id,
           req->client, id);
}

void wait_requests(struct client *client, struct request *reqs,
                   size_t num_reqs) {
  for (size_t i = 0; i < num_reqs; ++i)
    wait_request(client, &reqs[i]);
}

void place_requests(struct client *client) {
  // Allocate Buffer
  double *A = shm_malloc(1024 * sizeof(int));

  if (client->chatty)
    printf("Client(%d): Received Pointer to Allocated Memory: %p\n", client->id,
           (void *)A);

  // Write to Buffer
  printf("Client(%d): Writing Array:\n", client->id);

  double *input = shm_malloc(1024 * sizeof(double));
  for (size_t i = 0; i < 1024; ++i) {
    input[i] = (double)(2 * i);
  }

  struct request req1;
  scoria_write(client, A, 1024, input, NULL, NULL, 0, 0, &req1);
  wait_request(client, &req1);
  shm_free(input);

  // Read from Buffer
  printf("Client(%d): Reading Array:\n", client->id);

  double *output = shm_malloc(1024 * sizeof(double));

  struct request req2;
  scoria_read(client, A, 1024, output, NULL, NULL, 0, 0, &req2);
  wait_request(client, &req2);

  for (size_t i = 0; i < 1024; ++i)
    printf("%.2f ", output[i]);
  printf("\n");
  shm_free(output);

  // Free Buffer
  shm_free(A);

  // Exit Program
  // struct request req3;
  // scoria_quit(client, &req3);

  // wait_request(client, &req3);
}
