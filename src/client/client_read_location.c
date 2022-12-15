#include "client_read_location.h"

#include "client.h"
#include "config.h"

#include <stdio.h>

void read_location(struct client *client) {
  if (client->chatty)
    printf("Client: Waiting on Controller\n");

  while (!client->shared_location->ready) {
    ;
    ;
  }

  client->shared_mem_ptr = client->shared_location->shared_mem_ptr;
  client->shared_requests_list = client->shared_location->shared_requests_list;
  client->shared_completions_list =
      client->shared_location->shared_completions_list;

  if (client->chatty) {
    printf("Client: Received Shared Memory Addresses\n");
    printf("Client: shared_mem_ptr %p %p\n", (void *)client->shared_mem_ptr,
           (void *)client->shared_location->shared_mem_ptr);
    printf("Client: shared_requests_list %p %p\n",
           (void *)client->shared_requests_list,
           (void *)client->shared_location->shared_requests_list);
    printf("Client: shared_completions_list %p %p\n",
           (void *)client->shared_completions_list,
           (void *)client->shared_location->shared_completions_list);
  }
}
