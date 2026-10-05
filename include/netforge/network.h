#ifndef NETFORGE_NETWORK_H
#define NETFORGE_NETWORK_H

#include <stddef.h>
#include <stdint.h>
#include "netforge/common.h"

typedef struct {
    char address[NF_MAX_HOST];
    int responsive;
    int latency_ms;
    uint16_t port;
} nf_host_result_t;

int nf_network_parse_cidr(const char *cidr, uint32_t *network, unsigned *prefix);
int nf_network_discover(const char *cidr, uint16_t port, int timeout_ms, unsigned workers,
                        nf_host_result_t **results, size_t *count);
void nf_network_free_results(nf_host_result_t *results);

#endif
