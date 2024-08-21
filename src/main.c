#include "proxy.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <port> <thread_count> <cache_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int thread_count = atoi(argv[2]);
    int cache_size = atoi(argv[3]);

    if (port <= 0) {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }

    if (thread_count <= 0) {
        fprintf(stderr, "Invalid thread count\n");
        exit(EXIT_FAILURE);
    }

    if (cache_size <= 0) {
        fprintf(stderr, "Invalid cache size\n");
        exit(EXIT_FAILURE);
    }

    proxy_server_start(port, thread_count, cache_size);

    return 0;
}
