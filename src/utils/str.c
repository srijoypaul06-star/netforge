#define _POSIX_C_SOURCE 200809L
#include "netforge/str.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

bool nf_parse_u16(const char *s, uint16_t *out) {
    if (!s || !*s || s[0] == '-') return false;
    char *end = NULL; errno = 0;
    unsigned long v = strtoul(s, &end, 10);
    if (errno || *end || v > 65535UL) return false;
    *out = (uint16_t)v; return true;
}

bool nf_parse_int(const char *s, int *out) {
    if (!s || !*s) return false;
    char *end = NULL; errno = 0;
    long v = strtol(s, &end, 10);
    if (errno || *end || v < INT_MIN || v > INT_MAX) return false;
    *out = (int)v; return true;
}

static int append_port(uint16_t **arr, size_t *n, uint16_t p) {
    uint16_t *tmp = realloc(*arr, (*n + 1U) * sizeof(**arr));
    if (!tmp) return -1;
    *arr = tmp; (*arr)[*n] = p; (*n)++;
    return 0;
}

int nf_split_ports(const char *input, uint16_t **ports, size_t *count) {
    *ports = NULL; *count = 0;
    char *copy = strdup(input);
    if (!copy) return -1;
    for (char *tok = strtok(copy, ","); tok; tok = strtok(NULL, ",")) {
        while (isspace((unsigned char)*tok)) tok++;
        char *dash = strchr(tok, '-');
        if (!dash) {
            uint16_t p; if (!nf_parse_u16(tok, &p) || p == 0) goto fail;
            if (append_port(ports, count, p)) goto fail;
        } else {
            *dash = '\0'; uint16_t a, b;
            if (!nf_parse_u16(tok, &a) || !nf_parse_u16(dash + 1, &b) || a == 0 || b < a) goto fail;
            if ((unsigned)b - (unsigned)a > 4096U) goto fail;
            for (unsigned p = a; p <= b; ++p) if (append_port(ports, count, (uint16_t)p)) goto fail;
        }
    }
    free(copy); return *count ? 0 : -1;
fail:
    free(copy); free(*ports); *ports = NULL; *count = 0; return -1;
}
