#include <stdio.h>

#include "client.h"
#include "config.h"

#include "client_cleanup.h"
#include "client_init.h"
#include "client_place_requests.h"

int main(int argc, char **argv) {
  // Suppress Compiler Warnings
  (void)argc;
  (void)argv;

  struct client client;
  client.chatty = 0;

  init(&client);
  place_requests(&client);

  cleanup(&client);

  printf("Exiting...\n");

  return 0;
}
