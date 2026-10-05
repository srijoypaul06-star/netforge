#include "netforge/app.h"
#include "netforge/http.h"
#include "netforge/output.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int nf_cmd_http(int argc,char**argv,const nf_config_t*cfg){if(argc<2){fprintf(stderr,"usage: netforge http <host> [port]\n");return NF_ERR_USAGE;}int port=80;if(argc>2)port=atoi(argv[2]);nf_http_result_t r;if(nf_http_probe(argv[1],port,cfg->timeout_ms,&r)){fprintf(stderr,"HTTP probe failed\n");return NF_ERR_NETWORK;}if(nf_output_mode()==NF_OUTPUT_JSON){printf("{\"host\":");nf_output_json_escape(stdout,r.final_host);printf(",\"port\":%d,\"status\":%d,\"latency_ms\":%d,\"server\":",port,r.status_code,r.latency_ms);nf_output_json_escape(stdout,r.server);printf(",\"content_type\":");nf_output_json_escape(stdout,r.content_type);printf(",\"content_length\":%ld}\n",r.content_length);}else printf("HTTP %s:%d\n  status: %d\n  latency: %d ms\n  server: %s\n  content-type: %s\n  content-length: %ld\n",r.final_host,port,r.status_code,r.latency_ms,r.server[0]?r.server:"-",r.content_type[0]?r.content_type:"-",r.content_length);return 0;}
