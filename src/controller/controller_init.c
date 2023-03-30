#include "controller_init.h"
#include "controller_write_location.h"

#include "config.h"
#include "controller.h"
#include "posix_sm.h"
#include "request.h"
#include "utils.h"

#include "shm_malloc.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

void scoria_controller_init_files() {
  if (access(SHARED_MEMORY_NAME, F_OK) == 0)
    if (unlink(SHARED_MEMORY_NAME) == -1)
      scoria_error("controller:unlink:shared_memory");
}

void scoria_controller_init_memory_pool(struct controller *controller) {
  if (shm_init(SHARED_MEMORY_NAME, setup) < 0)
    scoria_error("Controller:shm_init");

  controller->shared_mem_ptr = shm_global();
}

void scoria_controller_init_requests(struct controller *controller) {
  controller->fd_requests =
      scoria_sm_open(SHARED_REQUESTS_NAME, O_RDWR | O_CREAT | O_TRUNC, 0660,
                     "controller:shm_open");
  scoria_sm_truncate(controller->fd_requests, sizeof(struct request_queue_list),
                     "controller:ftruncate");
  controller->shared_requests_list = scoria_sm_map(
      NULL, sizeof(struct request_queue_list), PROT_READ | PROT_WRITE,
      MAP_SHARED, controller->fd_requests, 0, "controller:mmap");

  request_queue_list_init(controller->shared_requests_list);

  if (controller->chatty)
    printf("Controller: Shared Request Address: %p\n",
           (void *)controller->shared_requests_list);
}

void scoria_controller_init_completions(struct controller *controller) {
  controller->fd_completions =
      scoria_sm_open(SHARED_COMPLETIONS_NAME, O_RDWR | O_CREAT | O_TRUNC, 0660,
                     "controller:shm_open");
  scoria_sm_truncate(controller->fd_completions,
                     sizeof(struct request_queue_list), "controller:ftruncate");
  controller->shared_completions_list = scoria_sm_map(
      NULL, sizeof(struct request_queue_list), PROT_READ | PROT_WRITE,
      MAP_SHARED, controller->fd_completions, 0, "controller:mmap");

  request_queue_list_init(controller->shared_completions_list);

  if (controller->chatty)
    printf("Controller: Shared Completions Address: %p\n",
           (void *)controller->shared_completions_list);
}

void scoria_controller_init_virtual_address_mailbox(
    struct controller *controller) {
  controller->fd_location =
      scoria_sm_open(SHARED_LOCATION_NAME, O_RDWR | O_CREAT | O_TRUNC, 0660,
                     "controller:shm_open");
  scoria_sm_truncate(controller->fd_location, sizeof(struct memory_location),
                     "controller:ftruncate");
  controller->shared_location = scoria_sm_map(
      NULL, sizeof(struct memory_location), PROT_READ | PROT_WRITE, MAP_SHARED,
      controller->fd_location, 0, "controller:mmap");

  controller->shared_location->ready = 0;
  controller->shared_location->shared_mem_ptr = NULL;
  controller->shared_location->shared_requests_list = NULL;
  controller->shared_location->shared_completions_list = NULL;

  if (controller->chatty)
    printf("Controller: Shared Location: %p\n",
           (void *)controller->shared_location);
}

void scoria_controller_init(struct controller *controller) {
  scoria_controller_init_files();

  scoria_controller_init_virtual_address_mailbox(controller);

  scoria_controller_init_memory_pool(controller);
  scoria_controller_init_requests(controller);
  scoria_controller_init_completions(controller);

  scoria_controller_write_location(controller);
}
