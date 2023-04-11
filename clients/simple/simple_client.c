#include <stdio.h>

#include "scoria.h"

void place_requests(struct client *client) {
  // Allocate Buffer
  double *A = shm_malloc(1024 * sizeof(double));

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
  scoria_wait_request(client, &req1);
  shm_free(input);

  // Read from Buffer
  printf("Client(%d): Reading Array:\n", client->id);

  double *output = shm_malloc(1024 * sizeof(double));

  struct request req2;
  scoria_read(client, A, 1024, output, NULL, NULL, 0, NONE, &req2);
  scoria_wait_request(client, &req2);

  for (size_t i = 0; i < 1024; ++i)
    printf("%.2f ", output[i]);
  printf("\n");
  shm_free(output);

  // Free Buffer
  shm_free(A);

  // Exit Program
  // struct request req3;
  // scoria_quit(client, &req3);

  // scoria_wait_request(client, &req3);
}

int main(int argc, char **argv) {
  // Suppress Compiler Warnings
  (void)argc;
  (void)argv;

  struct client client;
  client.chatty = 0;

  scoria_init(&client);
  place_requests(&client);

  scoria_cleanup(&client);

  printf("Exiting...\n");

  return 0;
}
