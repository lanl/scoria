#include "client_init.h"
#include "client_read_location.h"

#include "client.h"
#include "config.h"
#include "posix_sm.h"
#include "request.h"
#include "utils.h"

#include "shm_malloc.h"

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>

void scoria_client_init_memory_pool(struct client *client) {
  if (shm_init(SHARED_MEMORY_NAME, setup) < 0)
    scoria_error("Client:shm_init");

  client->shared_mem_ptr = shm_global();

  if (client->chatty) {
    printf("Client: Mapped Shared Memory Address: %p %p\n",
           (void *)client->shared_mem_ptr,
           (void *)client->shared_location->shared_mem_ptr);

    if (client->shared_mem_ptr == client->shared_location->shared_mem_ptr)
      printf("Client: Successfully Mapped Shared Memory Address\n");
    else
      // TODO: Error handling
      scoria_error("Client: Mapped Shared Memory to Incorrect Address\n");
  }
}

void scoria_client_init_requests(struct client *client) {
  client->fd_requests =
      scoria_sm_open(SHARED_REQUESTS_NAME, O_RDWR, 0, "client:shm_open");
  client->shared_requests_list =
      scoria_sm_map(client->shared_location->shared_requests_list,
                    sizeof(struct request_queue_list), PROT_READ | PROT_WRITE,
                    MAP_SHARED, client->fd_requests, 0, "client:mmap");

  if (client->chatty) {
    if (client->shared_requests_list ==
        client->shared_location->shared_requests_list)
      printf("Client: Successfully Mapped Shared Request Queue List to "
             "Address: %p %p\n",
             (void *)client->shared_requests_list,
             (void *)client->shared_location->shared_requests_list);
    else
      // TODO: Error handling
      scoria_error(
          "Client: Mapped Shared Request Queue List to Incorrect Address\n");
  }
}

void scoria_client_init_completions(struct client *client) {
  client->fd_completions =
      scoria_sm_open(SHARED_COMPLETIONS_NAME, O_RDWR, 0, "client:shm_open");
  client->shared_completions_list =
      scoria_sm_map(client->shared_location->shared_completions_list,
                    sizeof(struct request_queue_list), PROT_READ | PROT_WRITE,
                    MAP_SHARED, client->fd_completions, 0, "client:mmap");

  if (client->chatty) {
    if (client->shared_completions_list ==
        client->shared_location->shared_completions_list)
      printf("Client: Successfully Mapped Shared Completion Queue List to "
             "Address: %p "
             "%p\n",
             (void *)client->shared_completions_list,
             (void *)client->shared_location->shared_completions_list);
    else
      // TODO: Error handling
      scoria_error("Client: Mapped Shared Completiong Queue List to Incorrect "
                   "Address\n");
  }
}

void scoria_client_init_virtual_address_mailbox(struct client *client) {
  client->fd_location =
      scoria_sm_open(SHARED_LOCATION_NAME, O_RDWR, 0, "client:shm_open");
  client->shared_location = scoria_sm_map(
      NULL, sizeof(struct memory_location), PROT_READ | PROT_WRITE, MAP_SHARED,
      client->fd_location, 0, "client:mmap");
}

void scoria_client_init_id(struct client *client) {
  int id = -1;

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (client->shared_requests_list->queues[i].active == 0) {
      assert(client->shared_requests_list->queues[i].client == -1);

      assert(client->shared_completions_list->queues[i].active == 0);
      assert(client->shared_completions_list->queues[i].client == -1);

      // TODO: Thread Safe
      request_queue_activate(&(client->shared_requests_list->queues[i]), i);
      request_queue_activate(&(client->shared_completions_list->queues[i]), i);

      client->shared_requests = &(client->shared_requests_list->queues[i]);
      client->shared_completions =
          &(client->shared_completions_list->queues[i]);

      id = i;
      break;
    }
  }

  if (id == -1)
    scoria_error("Client: Exceeded Maxmimum Number of Clients\n");

  client->id = id;

  if (client->chatty)
    printf("Client: Assigned ID %d\n", client->id);
}

void scoria_init(struct client *client) {
  scoria_client_init_virtual_address_mailbox(client);

  scoria_client_read_location(client);

  scoria_client_init_memory_pool(client);
  scoria_client_init_requests(client);
  scoria_client_init_completions(client);

  scoria_client_init_id(client);

  client->unmatched_requests = NULL;

  printf("Client(%d): Connected to Controller Successfully\n", client->id);
}
