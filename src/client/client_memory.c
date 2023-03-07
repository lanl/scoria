#include "client_memory.h"

#include "client.h"
#include "config.h"
#include "request.h"

#include <semaphore.h>
#include <stdio.h>

static int rid = 0;

void scoria_put_request(struct client *client, struct request *req) {
  request_queue_put(client->shared_requests, req);

  if (client->chatty)
    printf("Client(%d): Added Request %d:%d to Request Queue %d\n", client->id,
           req->client, req->id, client->id);
}

void scoria_quit(struct client *client, struct request *req) {
  if (client->chatty)
    printf("Client(%d): Quit Request\n", client->id);

  req->client = client->id;
  req->r_type = Quit;
  req->id = rid;
  rid++;

  if (client->chatty)
    printf("Client(%d): Created Request Object: Client: %d ID: %d Type: %d\n",
           client->id, req->client, req->id, req->r_type);

  scoria_put_request(client, req);
}

void scoria_read(struct client *client, const void *buffer, const size_t N,
                 void *output, const size_t *ind1, const size_t *ind2,
                 size_t num_threads, i_type intrinsics, struct request *req) {
  if (client->chatty)
    printf("Client(%d): Reading Buffer\n", client->id);

  req->client = client->id;
  req->r_type = Read;
  req->nsecs = 0.0;
  req->input = buffer;
  req->output = output;
  req->N = N;
  req->ind1 = ind1;
  req->ind2 = ind2;
  req->nthreads = num_threads;
  req->intrinsics = intrinsics;
  req->id = rid;
  rid++;

  if (client->chatty)
    printf("Client(%d): Create Request Object: Client: %d ID: %d Type: %d\n",
           client->id, req->client, req->id, req->r_type);

  scoria_put_request(client, req);
}

void scoria_write(struct client *client, void *buffer, const size_t N,
                  const void *input, const size_t *ind1, const size_t *ind2,
                  size_t num_threads, i_type intrinsics, struct request *req) {
  if (client->chatty)
    printf("Client(%d): Writing Buffer\n", client->id);

  req->client = client->id;
  req->r_type = Write;
  req->nsecs = 0.0;
  req->output = buffer;
  req->input = input;
  req->N = N;
  req->ind1 = ind1;
  req->ind2 = ind2;
  req->nthreads = num_threads;
  req->intrinsics = intrinsics;
  req->id = rid;
  rid++;

  if (client->chatty)
    printf("Client(%d): Created Request Object: Client: %d ID: %d Type: %d\n",
           client->id, req->client, req->id, req->r_type);

  scoria_put_request(client, req);
}

void scoria_writeadd(struct client *client, void *buffer, const size_t N,
                     const void *input, const size_t *ind1, const size_t *ind2,
                     size_t num_threads, i_type intrinsics,
                     struct request *req) {
  if (client->chatty)
    printf("Client(%d): Writing and Adding Buffer\n", client->id);

  req->client = client->id;
  req->r_type = WriteAdd;
  req->nsecs = 0.0;
  req->output = buffer;
  req->input = input;
  req->N = N;
  req->ind1 = ind1;
  req->ind2 = ind2;
  req->nthreads = num_threads;
  req->intrinsics = intrinsics;
  req->id = rid;
  rid++;

  if (client->chatty)
    printf("Client(%d): Created Request Object: Client: %d ID: %d Type: %d\n",
           client->id, req->client, req->id, req->r_type);

  scoria_put_request(client, req);
}
