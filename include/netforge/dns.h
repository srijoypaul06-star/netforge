#ifndef NETFORGE_DNS_H
#define NETFORGE_DNS_H

#include <stddef.h>
#include <netdb.h>
#include "netforge/common.h"

typedef struct {
    char address[NF_MAX_HOST];
    int family;
} nf_dns_record_t;

int nf_dns_resolve(const char *host, nf_dns_record_t **records, size_t *count);
void nf_dns_free(nf_dns_record_t *records);
const char *nf_dns_family_name(int family);

#endif
