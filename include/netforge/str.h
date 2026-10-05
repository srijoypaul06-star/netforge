#ifndef NETFORGE_STR_H
#define NETFORGE_STR_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool nf_parse_u16(const char *s, uint16_t *out);
bool nf_parse_int(const char *s, int *out);
int nf_split_ports(const char *input, uint16_t **ports, size_t *count);

#endif
