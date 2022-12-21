#ifndef CONTROLLER_INIT
#define CONTROLLER_INIT

#include "controller.h"
#include "request.h"

void init_files();
void init_memory_pool(struct controller *controller);
void init_requests(struct controller *controller);
void init_completions(struct controller *controller);
void init(struct controller *controller);

#endif /* CONTROLLER_INIT */
