#ifndef CLIENT_WAIT_REQUESTS
#define CLIENT_WAIT_REQUESTS

#include "client.h"
#include "request.h"

void scoria_wait_request(struct client *client, struct request *req);
void scoria_wait_requests(struct client *client, struct request *reqs,
                          size_t num_reqs);

#endif /* CLIENT_WAIT_REQUESTS */
