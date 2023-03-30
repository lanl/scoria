#ifndef CLIENT_INIT
#define CLIENT_INIT

#include "client.h"
#include "request.h"

void scoria_client_init_memory_pool(struct client *client);
void scoria_client_init_requests(struct client *client);
void scoria_client_init_completions(struct client *client);
void scoria_client_init_id(struct client *client);
void scoria_init(struct client *client);

#endif /* CLIENT_INIT */
