#include "utils.h"
#include "config.h"

#include "shm_malloc.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

void setup() {
  struct shared_memory *shared_mem_ptr =
      shm_malloc(sizeof(struct shared_memory));

  if (!shared_mem_ptr) {
    // TODO: Handle Error
  }

  shared_mem_ptr->head = 0;
  shared_mem_ptr->tail = &shared_mem_ptr->head;

  shm_set_global(shared_mem_ptr);
}

void scoria_error(const char *msg) {
  perror(msg);
  exit(1);
}
