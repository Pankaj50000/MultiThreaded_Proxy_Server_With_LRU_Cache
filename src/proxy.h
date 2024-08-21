#ifndef PROXY_H
#define PROXY_H

#include "lru_cache.h"

void handle_request(int client_socket, lru_cache_t *cache);
void proxy_server_start(int port, int thread_count, int cache_size);

#endif /* PROXY_H */
