#include "client_wait_requests.h"
#include "client_memory.h"

#include "client.h"
#include "config.h"
#include "request.h"

#include "shm_malloc.h"
#include "uthash.h"

#include <stdio.h>

void scoria_wait_request(struct client *client, struct request *req) {
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

void scoria_wait_requests(struct client *client, struct request *reqs,
                          size_t num_reqs) {
  for (size_t i = 0; i < num_reqs; ++i)
    scoria_wait_request(client, &reqs[i]);
}
