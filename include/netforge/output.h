#ifndef NETFORGE_OUTPUT_H
#define NETFORGE_OUTPUT_H

#include <stdbool.h>
#include <stdio.h>

typedef enum { NF_OUTPUT_TABLE, NF_OUTPUT_JSON } nf_output_mode_t;
void nf_output_set_mode(nf_output_mode_t mode);
nf_output_mode_t nf_output_mode(void);
void nf_output_json_escape(FILE *out, const char *s);

#endif
