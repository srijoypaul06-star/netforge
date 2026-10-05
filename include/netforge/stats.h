#ifndef NETFORGE_STATS_H
#define NETFORGE_STATS_H
#include "netforge/tcp.h"
typedef struct { size_t total, open, closed, errors; double avg_ms; int min_ms, max_ms, p50_ms, p95_ms; } nf_scan_stats_t;
void nf_scan_stats(const nf_tcp_result_t *r, size_t n, nf_scan_stats_t *out);
#endif
