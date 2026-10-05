#ifndef NETFORGE_EPOLL_SCAN_H
#define NETFORGE_EPOLL_SCAN_H
#include "netforge/tcp.h"
int nf_epoll_scan(const char *host, const uint16_t *ports, size_t count,
                  int timeout_ms, unsigned max_inflight, nf_tcp_result_t *results);
#endif
