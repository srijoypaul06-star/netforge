#ifndef NETFORGE_APP_H
#define NETFORGE_APP_H

#include "netforge/config.h"
#include "netforge/output.h"

int nf_app_run(int argc, char **argv, const nf_config_t *cfg);
void nf_app_usage(const char *prog);

#endif
