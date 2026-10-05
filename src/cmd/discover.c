#include "netforge/app.h"
#include "netforge/network.h"
#include "netforge/output.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int nf_cmd_discover(int argc, char **argv, const nf_config_t *cfg) {
    if (argc < 2) {
        fprintf(stderr, "usage: netforge discover <CIDR> [port]\n  example: netforge discover 192.168.1.0/24 443\n");
        return NF_ERR_USAGE;
    }
    uint16_t port = 80;
    if (argc >= 3) {
        char *end = NULL;
        long value = strtol(argv[2], &end, 10);
        if (!*argv[2] || *end || value < 1 || value > 65535) {
            fprintf(stderr, "invalid port\n");
            return NF_ERR_USAGE;
        }
        port = (uint16_t)value;
    }

    nf_host_result_t *results = NULL;
    size_t count = 0;
    int rc = nf_network_discover(argv[1], port, cfg->timeout_ms, cfg->workers, &results, &count);
    if (rc != NF_OK) {
        fprintf(stderr, "invalid or unsupported network range\n");
        return rc;
    }

    size_t live = 0;
    for (size_t i = 0; i < count; ++i) if (results[i].responsive) ++live;

    if (nf_output_mode() == NF_OUTPUT_JSON) {
        printf("{\"network\":");
        nf_output_json_escape(stdout, argv[1]);
        printf(",\"port\":%u,\"hosts\":[", (unsigned)port);
        int first = 1;
        for (size_t i = 0; i < count; ++i) {
            if (!results[i].responsive) continue;
            if (!first) putchar(',');
            first = 0;
            printf("{\"address\":");
            nf_output_json_escape(stdout, results[i].address);
            printf(",\"latency_ms\":%d}", results[i].latency_ms);
        }
        printf("]}\n");
    } else {
        printf("\nNetForge Network Discovery — %s\n\n", argv[1]);
        printf("Probe: TCP/%u\n\n%-18s %-12s %s\n", (unsigned)port, "ADDRESS", "STATUS", "LATENCY");
        printf("------------------------------------------------\n");
        for (size_t i = 0; i < count; ++i) {
            if (!results[i].responsive) continue;
            printf("%-18s %-12s %d ms\n", results[i].address, "responsive", results[i].latency_ms);
        }
        printf("\nHosts: %zu | Responsive: %zu | Silent: %zu\n", count, live, count - live);
    }

    nf_network_free_results(results);
    return NF_OK;
}
