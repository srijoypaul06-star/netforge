#define _POSIX_C_SOURCE 200112L
#include "netforge/network.h"
#include "netforge/tcp.h"
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

int nf_network_parse_cidr(const char *cidr, uint32_t *network, unsigned *prefix) {
    if (!cidr || !network || !prefix) return NF_ERR_USAGE;
    char buf[64];
    if (strlen(cidr) >= sizeof(buf)) return NF_ERR_USAGE;
    strcpy(buf, cidr);
    char *slash = strchr(buf, '/');
    if (!slash) return NF_ERR_USAGE;
    *slash = '\0';
    char *end = NULL;
    long p = strtol(slash + 1, &end, 10);
    if (!*buf || !*(slash + 1) || *end != '\0' || p < 0 || p > 32) return NF_ERR_USAGE;
    struct in_addr addr;
    if (inet_pton(AF_INET, buf, &addr) != 1) return NF_ERR_USAGE;
    uint32_t host = ntohl(addr.s_addr);
    uint32_t mask = p == 0 ? 0U : 0xFFFFFFFFU << (32U - (unsigned)p);
    *network = host & mask;
    *prefix = (unsigned)p;
    return NF_OK;
}

typedef struct {
    uint32_t network;
    uint32_t first;
    uint32_t last;
    uint16_t port;
    int timeout;
    nf_host_result_t *results;
    size_t count;
    size_t next;
    pthread_mutex_t lock;
} discover_ctx_t;

static void format_ipv4(uint32_t host, char *out, size_t out_size) {
    struct in_addr addr = {.s_addr = htonl(host)};
    inet_ntop(AF_INET, &addr, out, (socklen_t)out_size);
}

static void *discover_worker(void *arg) {
    discover_ctx_t *ctx = arg;
    for (;;) {
        pthread_mutex_lock(&ctx->lock);
        size_t idx = ctx->next++;
        pthread_mutex_unlock(&ctx->lock);
        if (idx >= ctx->count) break;

        uint32_t host = ctx->first + (uint32_t)idx;
        char address[NF_MAX_HOST];
        format_ipv4(host, address, sizeof(address));
        nf_tcp_result_t probe;
        nf_tcp_probe(address, ctx->port, ctx->timeout, &probe);

        snprintf(ctx->results[idx].address, sizeof(ctx->results[idx].address), "%s", address);
        ctx->results[idx].responsive = probe.status == 1;
        ctx->results[idx].latency_ms = probe.latency_ms;
        ctx->results[idx].port = ctx->port;
    }
    return NULL;
}

int nf_network_discover(const char *cidr, uint16_t port, int timeout_ms, unsigned workers,
                        nf_host_result_t **results, size_t *count) {
    if (!results || !count || timeout_ms <= 0 || port == 0) return NF_ERR_USAGE;
    *results = NULL;
    *count = 0;

    uint32_t network;
    unsigned prefix;
    int rc = nf_network_parse_cidr(cidr, &network, &prefix);
    if (rc != NF_OK) return rc;

    uint64_t total = 1ULL << (32U - prefix);
    uint32_t first = network;
    uint32_t last = network + (uint32_t)(total - 1ULL);
    if (prefix <= 30U && total >= 2ULL) {
        first++;
        last--;
    }
    uint64_t host_count = last >= first ? (uint64_t)last - first + 1ULL : 0ULL;
    if (host_count == 0 || host_count > 65536ULL) return NF_ERR_USAGE;

    nf_host_result_t *items = calloc((size_t)host_count, sizeof(*items));
    if (!items) return NF_ERR_MEMORY;

    if (workers == 0) workers = 1;
    if (workers > NF_MAX_THREADS) workers = NF_MAX_THREADS;
    if ((uint64_t)workers > host_count) workers = (unsigned)host_count;

    discover_ctx_t ctx = {
        .network = network, .first = first, .last = last, .port = port,
        .timeout = timeout_ms, .results = items, .count = (size_t)host_count,
        .next = 0, .lock = PTHREAD_MUTEX_INITIALIZER
    };
    pthread_t *threads = calloc(workers, sizeof(*threads));
    if (!threads) { pthread_mutex_destroy(&ctx.lock); free(items); return NF_ERR_MEMORY; }

    unsigned made = 0;
    for (; made < workers; ++made) {
        if (pthread_create(&threads[made], NULL, discover_worker, &ctx) != 0) break;
    }
    for (unsigned i = 0; i < made; ++i) pthread_join(threads[i], NULL);
    free(threads);
    pthread_mutex_destroy(&ctx.lock);

    *results = items;
    *count = (size_t)host_count;
    return NF_OK;
}

void nf_network_free_results(nf_host_result_t *results) { free(results); }
