#include "client_cleanup.h"
#include "posix_sm.h"
#include "request.h"

#include "shm_malloc.h"

#include <unistd.h>

void cleanup_queues(struct client *client) {
  request_queue_deactivate(client->shared_requests);
  request_queue_deactivate(client->shared_completions);

  client->shared_requests = NULL;
  client->shared_completions = NULL;
}

void cleanup_shared_mem(struct client *client) {
  scoria_sm_unmap(client->shared_location, sizeof(struct memory_location),
              "client:unmap");

  close(client->fd_location);
  close(client->fd_requests);
  close(client->fd_completions);
}

void cleanup(struct client *client) {
  cleanup_queues(client);
  cleanup_shared_mem(client);
}
