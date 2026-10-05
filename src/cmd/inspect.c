#include "netforge/app.h"
#include "netforge/dns.h"
#include "netforge/http.h"
#include "netforge/output.h"
#include "netforge/tcp.h"
#include "netforge/str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const uint16_t default_ports[] = {22, 53, 80, 443, 3306, 5432, 6379, 8080, 8443};

int nf_cmd_inspect(int argc, char **argv, const nf_config_t *cfg) {
    if (argc < 2) {
        fprintf(stderr, "usage: netforge inspect <host> [ports]\n");
        fprintf(stderr, "  default ports: 22,53,80,443,3306,5432,6379,8080,8443\n");
        return NF_ERR_USAGE;
    }

    const char *host = argv[1];
    uint16_t *parsed = NULL;
    size_t port_count = 0;
    const uint16_t *ports = default_ports;
    if (argc >= 3) {
        if (nf_split_ports(argv[2], &parsed, &port_count) != 0) {
            fprintf(stderr, "invalid port specification\n");
            return NF_ERR_USAGE;
        }
        ports = parsed;
    } else {
        port_count = sizeof(default_ports) / sizeof(default_ports[0]);
    }

    nf_dns_record_t *records = NULL;
    size_t record_count = 0;
    const int dns_rc = nf_dns_resolve(host, &records, &record_count);

    nf_tcp_result_t *results = calloc(port_count, sizeof(*results));
    if (!results) {
        free(parsed);
        nf_dns_free(records);
        return NF_ERR_MEMORY;
    }
    const int scan_rc = nf_tcp_scan(host, ports, port_count, cfg->timeout_ms, cfg->workers, results);
    if (scan_rc != NF_OK) {
        free(results);
        free(parsed);
        nf_dns_free(records);
        return scan_rc;
    }

    nf_http_result_t http_results[2];
    int http_ports[2] = {80, 8080};
    size_t http_count = 0;
    for (size_t i = 0; i < port_count; ++i) {
        if (results[i].status != 1) continue;
        if (results[i].port == 80 || results[i].port == 8080) {
            if (nf_http_probe(host, (int)results[i].port, cfg->timeout_ms, &http_results[http_count]) == 0) {
                http_ports[http_count] = (int)results[i].port;
                ++http_count;
            }
        }
    }

    if (nf_output_mode() == NF_OUTPUT_JSON) {
        printf("{\"host\":");
        nf_output_json_escape(stdout, host);
        printf(",\"dns_status\":%s,\"addresses\":[", dns_rc == NF_OK ? "true" : "false");
        for (size_t i = 0; i < record_count; ++i) {
            if (i) putchar(',');
            printf("{\"type\":");
            nf_output_json_escape(stdout, nf_dns_family_name(records[i].family));
            printf(",\"address\":");
            nf_output_json_escape(stdout, records[i].address);
            putchar('}');
        }
        printf("],\"ports\":[");
        int first = 1;
        for (size_t i = 0; i < port_count; ++i) {
            if (!first) putchar(',');
            first = 0;
            printf("{\"port\":%u,\"status\":", (unsigned)results[i].port);
            nf_output_json_escape(stdout, nf_tcp_status_name(results[i].status));
            printf(",\"latency_ms\":%d,\"service\":", results[i].latency_ms);
            nf_output_json_escape(stdout, results[i].service);
            printf(",\"banner\":");
            nf_output_json_escape(stdout, results[i].banner);
            putchar('}');
        }
        printf("],\"http\":[");
        for (size_t i = 0; i < http_count; ++i) {
            if (i) putchar(',');
            printf("{\"port\":%d,\"status\":%d,\"latency_ms\":%d,\"server\":", http_ports[i], http_results[i].status_code, http_results[i].latency_ms);
            nf_output_json_escape(stdout, http_results[i].server);
            printf(",\"content_type\":");
            nf_output_json_escape(stdout, http_results[i].content_type);
            putchar('}');
        }
        printf("]}\n");
    } else {
        printf("\nNetForge Host Intelligence — %s\n\n", host);
        printf("DNS addresses:\n");
        if (dns_rc != NF_OK || record_count == 0) {
            printf("  unavailable\n");
        } else {
            for (size_t i = 0; i < record_count; ++i) {
                printf("  %-4s %s\n", nf_dns_family_name(records[i].family), records[i].address);
            }
        }
        printf("\nTCP services:\n");
        printf("%-8s %-10s %-10s %-16s %s\n", "PORT", "STATUS", "LATENCY", "SERVICE", "BANNER");
        printf("--------------------------------------------------------------------------\n");
        for (size_t i = 0; i < port_count; ++i) {
            printf("%-8u %-10s %-10d %-16s %s\n", (unsigned)results[i].port,
                   nf_tcp_status_name(results[i].status), results[i].latency_ms,
                   results[i].service, results[i].banner[0] ? results[i].banner : "-");
        }
        if (http_count) {
            printf("\nHTTP observations:\n");
            for (size_t i = 0; i < http_count; ++i) {
                printf("  %d → HTTP %d, %d ms, server=%s, type=%s\n", http_ports[i],
                       http_results[i].status_code, http_results[i].latency_ms,
                       http_results[i].server[0] ? http_results[i].server : "-",
                       http_results[i].content_type[0] ? http_results[i].content_type : "-");
            }
        }
    }

    free(results);
    free(parsed);
    nf_dns_free(records);
    return NF_OK;
}
