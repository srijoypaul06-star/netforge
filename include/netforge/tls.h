#ifndef NETFORGE_TLS_H
#define NETFORGE_TLS_H
#include <stddef.h>
typedef struct {
    int ok;
    int latency_ms;
    long verify_result;
    char protocol[32];
    char cipher[96];
    char subject[256];
    char issuer[256];
    char not_before[64];
    char not_after[64];
} nf_tls_result_t;
int nf_tls_probe(const char *host, int port, int timeout_ms, nf_tls_result_t *out);
#endif
