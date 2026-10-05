#ifndef NETFORGE_INTERFACES_H
#define NETFORGE_INTERFACES_H

#include <stddef.h>

typedef struct {
    char name[64];
    char address[128];
    char netmask[128];
    unsigned flags;
} nf_interface_t;

int nf_interfaces_list(nf_interface_t **items, size_t *count);
void nf_interfaces_free(nf_interface_t *items);

#endif
