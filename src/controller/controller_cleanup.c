#include "controller_cleanup.h"
#include "posix_sm.h"
#include "request.h"
#include "utils.h"

#include "shm_malloc.h"

#include <unistd.h>

void cleanup_shared_mem(struct controller *controller) {
  request_queue_list_free(controller->shared_requests_list);
  request_queue_list_free(controller->shared_completions_list);

  scoria_sm_unmap(controller->shared_location, sizeof(struct memory_location),
                  "controller:unmap:shared_location");
  scoria_sm_unlink(SHARED_LOCATION_NAME,
                   "controller:sem_unlink:shared_location");

  scoria_sm_unmap(controller->shared_requests_list,
                  sizeof(struct request_queue_list),
                  "controller:unmap:shared_requests");
  scoria_sm_unlink(SHARED_REQUESTS_NAME,
                   "controller:sem_unlink:shared_requests");

  scoria_sm_unmap(controller->shared_completions_list,
                  sizeof(struct request_queue_list),
                  "controller:unmap:shared_completions");
  scoria_sm_unlink(SHARED_COMPLETIONS_NAME,
                   "controller:sem_unlink:shared_completions");

  close(controller->fd_location);
  close(controller->fd_requests);
  close(controller->fd_completions);
}

void cleanup(struct controller *controller) {
  cleanup_shared_mem(controller);

  shm_destroy();
}
