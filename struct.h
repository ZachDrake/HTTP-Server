#pragma once

#define MAX_HEADER_COUNT 10
#define MAX_HEADER_SIZE 1024
#define MAX_BODY_SIZE 1024

typedef struct {
    char method[16];
    char path[256];
    char headers[MAX_HEADER_COUNT][MAX_HEADER_SIZE];
    int header_count;
    char body[MAX_BODY_SIZE];
} http_request_t;

