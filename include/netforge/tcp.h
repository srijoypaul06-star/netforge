#ifndef NETFORGE_TCP_H
#define NETFORGE_TCP_H

#include <stdint.h>
#include <stdbool.h>
#include "netforge/common.h"

typedef struct {
    uint16_t port;
    int status;
    int latency_ms;
    char service[NF_MAX_SERVICE];
    char banner[128];
} nf_tcp_result_t;

int nf_tcp_probe(const char *host, uint16_t port, int timeout_ms, nf_tcp_result_t *result);
int nf_tcp_scan(const char *host, const uint16_t *ports, size_t count,
                int timeout_ms, unsigned workers, nf_tcp_result_t *results);
const char *nf_tcp_status_name(int status);
const char *nf_tcp_guess_service(uint16_t port);

#endif
