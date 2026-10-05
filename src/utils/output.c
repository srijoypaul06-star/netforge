#include "netforge/output.h"
#include <string.h>

static nf_output_mode_t g_mode = NF_OUTPUT_TABLE;
void nf_output_set_mode(nf_output_mode_t mode) { g_mode = mode; }
nf_output_mode_t nf_output_mode(void) { return g_mode; }

void nf_output_json_escape(FILE *out, const char *s) {
    fputc('"', out);
    for (const unsigned char *p = (const unsigned char *)s; *p; ++p) {
        switch (*p) {
            case '"': fputs("\\\"", out); break;
            case '\\': fputs("\\\\", out); break;
            case '\n': fputs("\\n", out); break;
            case '\r': fputs("\\r", out); break;
            case '\t': fputs("\\t", out); break;
            default: fputc(*p, out); break;
        }
    }
    fputc('"', out);
}
