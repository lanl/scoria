#ifndef CLIENT_PLACE_REQUESTS
#define CLIENT_PLACE_REQUESTS

#include "client.h"
#include "request.h"

void wait_request(struct client *client, struct request *req);
void wait_requests(struct client *client, struct request *reqs,
                   size_t num_reqs);
void place_requests(struct client *client);

#endif /* CLIENT_PLACE_REQUESTS */
