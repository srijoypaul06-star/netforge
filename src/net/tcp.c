#define _POSIX_C_SOURCE 200112L
#include "netforge/tcp.h"
#include "netforge/log.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/socket.h>
#include <poll.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static long long now_ms(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}

const char *nf_tcp_guess_service(uint16_t port) {
    switch (port) {
        case 21: return "ftp"; case 22: return "ssh"; case 23: return "telnet";
        case 25: return "smtp"; case 53: return "dns"; case 80: return "http";
        case 110: return "pop3"; case 143: return "imap"; case 443: return "https";
        case 3306: return "mysql"; case 5432: return "postgres"; case 6379: return "redis";
        case 8080: return "http-alt"; case 8443: return "https-alt"; default: return "unknown";
    }
}

const char *nf_tcp_status_name(int status) {
    switch (status) { case 1: return "open"; case 0: return "closed"; default: return "error"; }
}

static void read_passive_banner(const char *host, uint16_t port, int timeout_ms, nf_tcp_result_t *result) {
    if (!(port == 21 || port == 22 || port == 25 || port == 110 || port == 143 || port == 587)) return;

    struct addrinfo hints = {0}, *res = NULL;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    char service[8];
    snprintf(service, sizeof(service), "%u", (unsigned)port);
    if (getaddrinfo(host, service, &hints, &res) != 0) return;

    for (struct addrinfo *rp = res; rp; rp = rp->ai_next) {
        int fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd < 0) continue;
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) { close(fd); continue; }

        int rc = connect(fd, rp->ai_addr, rp->ai_addrlen);
        if (rc < 0 && errno != EINPROGRESS) { close(fd); continue; }
        if (rc < 0) {
            struct pollfd pfd = {.fd = fd, .events = POLLOUT, .revents = 0};
            if (poll(&pfd, 1, timeout_ms) <= 0 || !(pfd.revents & POLLOUT)) { close(fd); continue; }
            int error = 0;
            socklen_t len = sizeof(error);
            if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) != 0 || error != 0) { close(fd); continue; }
        }

        struct pollfd pfd = {.fd = fd, .events = POLLIN, .revents = 0};
        if (poll(&pfd, 1, timeout_ms < 250 ? timeout_ms : 250) > 0 && (pfd.revents & POLLIN)) {
            char banner[sizeof(result->banner)];
            ssize_t n = recv(fd, banner, sizeof(banner) - 1U, 0);
            if (n > 0) {
                banner[n] = '\0';
                for (ssize_t i = 0; i < n; ++i) {
                    if (banner[i] == '\r' || banner[i] == '\n' || banner[i] == '\t') banner[i] = ' ';
                    if ((unsigned char)banner[i] < 32U) banner[i] = ' ';
                }
                snprintf(result->banner, sizeof(result->banner), "%s", banner);
            }
        }
        close(fd);
        if (result->banner[0] != '\0') break;
    }
    freeaddrinfo(res);
}

int nf_tcp_probe(const char *host, uint16_t port, int timeout_ms, nf_tcp_result_t *result) {
    if (!host || !result || timeout_ms <= 0) return NF_ERR_USAGE;
    memset(result, 0, sizeof(*result));
    result->port = port;
    snprintf(result->service, sizeof(result->service), "%s", nf_tcp_guess_service(port));

    struct addrinfo hints = {0}, *res = NULL;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    char service[8];
    snprintf(service, sizeof(service), "%u", (unsigned)port);
    int ga = getaddrinfo(host, service, &hints, &res);
    if (ga != 0) { result->status = -1; return NF_ERR_NETWORK; }

    long long start = now_ms();
    int final = 0;
    for (struct addrinfo *rp = res; rp; rp = rp->ai_next) {
        int fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd < 0) continue;
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) { close(fd); continue; }
        int rc = connect(fd, rp->ai_addr, rp->ai_addrlen);
        if (rc == 0) {
            final = 1;
            close(fd);
            break;
        }
        if (errno == EINPROGRESS) {
            struct pollfd pfd = {.fd = fd, .events = POLLOUT, .revents = 0};
            int pr = poll(&pfd, 1, timeout_ms);
            if (pr > 0 && (pfd.revents & POLLOUT)) {
                int error = 0;
                socklen_t len = sizeof(error);
                if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) == 0 && error == 0) final = 1;
            }
        }
        close(fd);
        if (final) break;
    }
    freeaddrinfo(res);
    result->latency_ms = (int)(now_ms() - start);
    result->status = final ? 1 : 0;
    if (final) read_passive_banner(host, port, timeout_ms, result);
    return NF_OK;
}

typedef struct { const char *host; const uint16_t *ports; nf_tcp_result_t *results; size_t count; size_t next; pthread_mutex_t lock; int timeout; } scan_ctx_t;
static void *worker(void *arg) {
    scan_ctx_t *ctx = arg;
    for (;;) {
        pthread_mutex_lock(&ctx->lock); size_t i = ctx->next++; pthread_mutex_unlock(&ctx->lock);
        if (i >= ctx->count) break;
        nf_tcp_probe(ctx->host, ctx->ports[i], ctx->timeout, &ctx->results[i]);
    }
    return NULL;
}

int nf_tcp_scan(const char *host, const uint16_t *ports, size_t count, int timeout_ms, unsigned workers, nf_tcp_result_t *results) {
    if (!count) return NF_ERR_USAGE;
    if (workers == 0) workers = 1;
    if (workers > NF_MAX_THREADS) workers = NF_MAX_THREADS;
    scan_ctx_t ctx = {host, ports, results, count, 0, PTHREAD_MUTEX_INITIALIZER, timeout_ms};
    pthread_t *threads = calloc(workers, sizeof(*threads)); if (!threads) return NF_ERR_MEMORY;
    unsigned made = 0;
    for (; made < workers; ++made) if (pthread_create(&threads[made], NULL, worker, &ctx) != 0) break;
    for (unsigned i = 0; i < made; ++i) pthread_join(threads[i], NULL);
    free(threads); pthread_mutex_destroy(&ctx.lock); return NF_OK;
}
