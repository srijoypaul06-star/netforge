#ifndef NETFORGE_COMMON_H
#define NETFORGE_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define NF_VERSION "2.0.0"
#define NF_MAX_HOST 256
#define NF_MAX_SERVICE 64
#define NF_MAX_LINE 1024
#define NF_DEFAULT_TIMEOUT_MS 1000
#define NF_MAX_THREADS 128

typedef enum {
    NF_OK = 0,
    NF_ERR = -1,
    NF_ERR_USAGE = -2,
    NF_ERR_MEMORY = -3,
    NF_ERR_NETWORK = -4,
    NF_ERR_TIMEOUT = -5,
    NF_ERR_NOT_FOUND = -6
} nf_status_t;

#endif
