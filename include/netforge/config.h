#ifndef NETFORGE_CONFIG_H
#define NETFORGE_CONFIG_H

#include "netforge/common.h"

typedef struct {
    int timeout_ms;
    unsigned workers;
    int verbose;
} nf_config_t;

void nf_config_default(nf_config_t *cfg);

#endif
