#ifndef CLIENT_INIT
#define CLIENT_INIT

#include "client.h"
#include "request.h"

void init_memory_pool(struct client *client);
void init_requests(struct client *client);
void init_completions(struct client *client);
void init_id(struct client *client);
void init(struct client *client);

#endif /* CLIENT_INIT */
