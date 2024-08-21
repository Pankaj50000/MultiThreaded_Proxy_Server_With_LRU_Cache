#include "http_parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int parse_http_request(const char *buffer, http_request_t *request) {
    // Check for NULL pointers
    if (buffer == NULL || request == NULL) {
        return -1; // Error code for invalid input
    }

    // Initialize the request structure to zeros
    memset(request, 0, sizeof(http_request_t));

    // Parse the request line: METHOD URL VERSION
    if (sscanf(buffer, "%15s %1023s %15s", request->method, request->url, request->version) != 3) {
        return -1; // Error code for parsing failure
    }

    // Extract the host from the request headers
    const char *host_start = strstr(buffer, "Host: ");
    if (host_start != NULL) {
        host_start += 6; // Skip "Host: "
        const char *host_end = strchr(host_start, '\r');
        if (host_end != NULL) {
            size_t host_length = host_end - host_start;
            if (host_length < sizeof(request->host)) {
                strncpy(request->host, host_start, host_length);
                request->host[host_length] = '\0'; // Null-terminate the host string
            } else {
                return -1; // Host header too long
            }
        }
    }

    // Extract the Connection header from the request
    const char *connection_start = strstr(buffer, "Connection: ");
    if (connection_start != NULL) {
        connection_start += 12; // Skip "Connection: "
        const char *connection_end = strchr(connection_start, '\r');
        if (connection_end != NULL) {
            size_t connection_length = connection_end - connection_start;
            if (connection_length < sizeof(request->connection)) {
                strncpy(request->connection, connection_start, connection_length);
                request->connection[connection_length] = '\0'; // Null-terminate the connection string
            } else {
                return -1; // Connection header too long
            }
        }
    }

    return 0; // Success
}
