#include <stdio.h>

#include "config.h"
#include "controller.h"

#include "controller_cleanup.h"
#include "controller_handle_requests.h"
#include "controller_init.h"

int main(int argc, char **argv) {
  // Suppress Compiler Warnings
  (void)argc;
  (void)argv;

  struct controller controller;
  controller.chatty = 1;

  init(&controller);
  handle_requests(&controller);

  cleanup(&controller);

  printf("Exiting...\n");

  return 0;
}
