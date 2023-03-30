#ifndef CONTROLLER_INIT
#define CONTROLLER_INIT

#include "controller.h"
#include "request.h"

void scoria_controller_init_files();
void scoria_controller_init_memory_pool(struct controller *controller);
void scoria_controller_init_requests(struct controller *controller);
void scoria_controller_init_completions(struct controller *controller);
void scoria_controller_init(struct controller *controller);

#endif /* CONTROLLER_INIT */
