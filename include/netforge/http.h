#ifndef NETFORGE_HTTP_H
#define NETFORGE_HTTP_H

#include <stddef.h>

typedef struct {
    int status_code;
    long content_length;
    int latency_ms;
    char server[128];
    char content_type[128];
    char final_host[256];
} nf_http_result_t;

int nf_http_probe(const char *host, int port, int timeout_ms, nf_http_result_t *result);

#endif
