#include <stdio.h>

#include "client.h"
#include "config.h"

#include "client_cleanup.h"
#include "client_init.h"
#include "client_memory.h"
#include "client_wait_requests.h"

#include "shm_malloc.h"

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
  scoria_write(client, A, 1024, input, NULL, NULL, 0, NONE, &req1);
  wait_request(client, &req1);
  shm_free(input);

  // Read from Buffer
  printf("Client(%d): Reading Array:\n", client->id);

  double *output = shm_malloc(1024 * sizeof(double));

  struct request req2;
  scoria_read(client, A, 1024, output, NULL, NULL, 0, NONE, &req2);
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

int main(int argc, char **argv) {
  // Suppress Compiler Warnings
  (void)argc;
  (void)argv;

  struct client client;
  client.chatty = 0;

  init(&client);
  place_requests(&client);

  cleanup(&client);

  printf("Exiting...\n");

  return 0;
}
