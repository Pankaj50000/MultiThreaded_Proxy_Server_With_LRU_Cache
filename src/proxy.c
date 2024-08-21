#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include "proxy.h"
#include "thread_pool.h"
#include "lru_cache.h"
#include "http_parser.h"

#define BACKLOG 10
#define BUFFER_SIZE 4096

typedef struct {
    int client_socket;
    lru_cache_t *cache;
} request_args_t;

void handle_request_wrapper(void *args) {
    request_args_t *request_args = (request_args_t *)args;
    if (request_args == NULL) {
        fprintf(stderr, "Request arguments are NULL\n");
        return;
    }

    handle_request(request_args->client_socket, request_args->cache);
    free(request_args);
}

void handle_request(int client_socket, lru_cache_t *cache) {
    if (cache == NULL) {
        fprintf(stderr, "Cache is NULL\n");
        close(client_socket);
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Read the HTTP request from the client
    bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) {
        perror("Error reading from socket");
        close(client_socket);
        return;
    }

    buffer[bytes_read] = '\0';

    http_request_t request;
    if (parse_http_request(buffer, &request) == -1) {
        fprintf(stderr, "Failed to parse HTTP request.\n");
        close(client_socket);
        return;
    }

    if (request.url[0] == '\0' || request.host[0] == '\0') {
        fprintf(stderr, "Request URL or Host is empty\n");
        close(client_socket);
        return;
    }

    char *cached_response = lru_cache_get(cache, request.url);
    if (cached_response != NULL) {
        write(client_socket, cached_response, strlen(cached_response));
    } else {
        int server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0) {
            perror("Error creating server socket");
            close(client_socket);
            return;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(80);

        struct hostent *server = gethostbyname(request.host);
        if (server == NULL) {
            fprintf(stderr, "No such host: %s\n", request.host);
            close(server_socket);
            close(client_socket);
            return;
        }

        memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

        if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Error connecting to server");
            close(server_socket);
            close(client_socket);
            return;
        }

        write(server_socket, buffer, bytes_read);

        char response[BUFFER_SIZE];
        ssize_t response_size = read(server_socket, response, sizeof(response) - 1);
        if (response_size < 0) {
            perror("Error reading from server socket");
            close(server_socket);
            close(client_socket);
            return;
        }

        response[response_size] = '\0';

        lru_cache_put(cache, request.url, response);
        write(client_socket, response, response_size);

        close(server_socket);
    }

    close(client_socket);
}

void proxy_server_start(int port, int thread_count, int cache_size) {
    int server_socket;
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, BACKLOG) < 0) {
        perror("Error listening on socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Proxy server started on port %d\n", port);

    int queue_size = 100;
    thread_pool_t *thread_pool = thread_pool_create(thread_count, queue_size);
    if (thread_pool == NULL) {
        fprintf(stderr, "Failed to create thread pool\n");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    lru_cache_t *cache = lru_cache_create(cache_size);
    if (cache == NULL) {
        fprintf(stderr, "Failed to create LRU cache\n");
        thread_pool_destroy(thread_pool);
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    while (1) {
        int client_socket;
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
            continue;
        }

        request_args_t *args = malloc(sizeof(request_args_t));
        if (args == NULL) {
            fprintf(stderr, "Failed to allocate memory for request arguments\n");
            close(client_socket);
            continue;
        }

        args->client_socket = client_socket;
        args->cache = cache;

        if (thread_pool_add(thread_pool, handle_request_wrapper, (void *)args) != 0) {
            fprintf(stderr, "Failed to add task to thread pool\n");
            free(args);
            close(client_socket);
            continue;
        }
    }

    // Clean up resources
    thread_pool_destroy(thread_pool);
    lru_cache_destroy(cache);
    close(server_socket);
}
