#include "netforge/config.h"
void nf_config_default(nf_config_t *cfg) {
    cfg->timeout_ms = NF_DEFAULT_TIMEOUT_MS;
    cfg->workers = 16U;
    cfg->verbose = 0;
}
