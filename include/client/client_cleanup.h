#ifndef CLIENT_CLEANUP
#define CLIENT_CLEANUP

#include "client.h"

void scoria_client_cleanup_queues(struct client *client);
void scoria_client_cleanup_shared_mem(struct client *client);
void scoria_cleanup(struct client *client);

#endif /* CLIENT_CLEANUP */
