#ifndef CLIENT_MEMORY_H
#define CLIENT_MEMORY_H

#include <semaphore.h>

#include "client.h"
#include "request.h"

void scoria_put_request(struct client *client, struct request *req);

void scoria_quit(struct client *client, struct request *req);
void scoria_read(struct client *client, void *buffer, const size_t N,
                 void *output, const size_t *ind1, const size_t *ind2,
                 size_t num_threads, bool use_avx, struct request *req);
void scoria_write(struct client *client, void *buffer, const size_t N,
                  void *input, const size_t *ind1, const size_t *ind2,
                  size_t num_threads, bool use_avx, struct request *req);

#endif /* CLIENT_MEMORY_H */
