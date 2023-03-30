#include "controller_write_location.h"

#include "config.h"
#include "controller.h"

#include <stdio.h>

void scoria_controller_write_location(struct controller *controller) {
  controller->shared_location->shared_mem_ptr = controller->shared_mem_ptr;
  controller->shared_location->shared_requests_list =
      controller->shared_requests_list;
  controller->shared_location->shared_completions_list =
      controller->shared_completions_list;

  controller->shared_location->ready = 1;

  if (controller->chatty) {
    printf("Controller: Posted Shared Memory Addresses\n");
    printf("Controller: shared_mem_ptr %p %p\n",
           (void *)controller->shared_location->shared_mem_ptr,
           (void *)controller->shared_mem_ptr);
    printf("Controller: shared_requests_list %p %p\n",
           (void *)controller->shared_location->shared_requests_list,
           (void *)controller->shared_requests_list);
    printf("Controller: shared_completions_list %p %p\n",
           (void *)controller->shared_location->shared_completions_list,
           (void *)controller->shared_completions_list);
  }
}
