#define _POSIX_C_SOURCE 200112L
#include "netforge/dns.h"
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>

const char *nf_dns_family_name(int family) { return family == AF_INET6 ? "AAAA" : family == AF_INET ? "A" : "OTHER"; }
int nf_dns_resolve(const char *host, nf_dns_record_t **records, size_t *count) {
    *records = NULL; *count = 0;
    struct addrinfo hints = {0}, *res = NULL; hints.ai_family = AF_UNSPEC; hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, NULL, &hints, &res) != 0) return NF_ERR_NOT_FOUND;
    for (struct addrinfo *p = res; p; p = p->ai_next) {
        if (p->ai_family != AF_INET && p->ai_family != AF_INET6) continue;
        nf_dns_record_t *tmp = realloc(*records, (*count + 1U) * sizeof(**records));
        if (!tmp) { freeaddrinfo(res); nf_dns_free(*records); *records=NULL; *count=0; return NF_ERR_MEMORY; }
        *records = tmp;
        char numeric_host[NF_MAX_HOST]; if (getnameinfo(p->ai_addr, p->ai_addrlen, numeric_host, sizeof(numeric_host), NULL, 0, NI_NUMERICHOST) != 0) continue;
        snprintf((*records)[*count].address, sizeof((*records)[*count].address), "%s", numeric_host);
        (*records)[*count].family = p->ai_family; (*count)++;
    }
    freeaddrinfo(res); return *count ? NF_OK : NF_ERR_NOT_FOUND;
}
void nf_dns_free(nf_dns_record_t *records) { free(records); }
