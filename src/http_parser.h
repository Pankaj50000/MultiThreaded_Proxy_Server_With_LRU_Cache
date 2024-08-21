#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#define METHOD_LEN 16
#define URL_LEN 1024
#define HOST_LEN 256
#define CONNECTION_LEN 32

typedef struct {
    char method[METHOD_LEN];
    char url[URL_LEN];
    char host[HOST_LEN];
    char connection[CONNECTION_LEN];
    char version[16];
} http_request_t;

int parse_http_request(const char *buffer, http_request_t *request);

#endif /* HTTP_PARSER_H */
