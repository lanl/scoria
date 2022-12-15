#ifndef CLIENT_CLEANUP
#define CLIENT_CLEANUP

#include "client.h"

void cleanup_queues(struct client *client);
void cleanup_shared_mem(struct client *client);
void cleanup(struct client *client);

#endif /* CLIENT_CLEANUP */
