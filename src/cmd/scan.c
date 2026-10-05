#include "netforge/app.h"
#include "netforge/tcp.h"
#include "netforge/epoll_scan.h"
#include "netforge/stats.h"
#include "netforge/str.h"
#include "netforge/output.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void csv_field(const char *s){putchar('"');for(;*s;s++){if(*s=='"')putchar('"');putchar(*s);}putchar('"');}
int nf_cmd_scan(int argc,char **argv,const nf_config_t *cfg){
 if(argc<3){fprintf(stderr,"usage: netforge scan <host> <ports> [--open-only] [--engine threads|epoll] [--csv]\n");return NF_ERR_USAGE;}
 const char *host=argv[1],*engine="threads";int open_only=0,csv=0;uint16_t *ports=NULL;size_t count=0;
 if(nf_split_ports(argv[2],&ports,&count)!=0){fprintf(stderr,"invalid port specification\n");return NF_ERR_USAGE;}
 for(int i=3;i<argc;i++){if(strcmp(argv[i],"--open-only")==0)open_only=1;else if(strcmp(argv[i],"--json")==0)nf_output_set_mode(NF_OUTPUT_JSON);else if(strcmp(argv[i],"--csv")==0)csv=1;else if(strcmp(argv[i],"--engine")==0&&i+1<argc)engine=argv[++i];}
 nf_tcp_result_t *r=calloc(count,sizeof(*r));if(!r){free(ports);return NF_ERR_MEMORY;}int rc;
 if(strcmp(engine,"epoll")==0)rc=nf_epoll_scan(host,ports,count,cfg->timeout_ms,cfg->workers,r);else if(strcmp(engine,"threads")==0)rc=nf_tcp_scan(host,ports,count,cfg->timeout_ms,cfg->workers,r);else{fprintf(stderr,"unknown engine: %s\n",engine);free(r);free(ports);return NF_ERR_USAGE;}
 if(rc!=NF_OK){free(r);free(ports);return rc;}nf_scan_stats_t st;nf_scan_stats(r,count,&st);
 if(csv){puts("host,port,status,latency_ms,service,banner");for(size_t i=0;i<count;i++){if(open_only&&r[i].status!=1)continue;csv_field(host);printf(",%u,",(unsigned)r[i].port);csv_field(nf_tcp_status_name(r[i].status));printf(",%d,",r[i].latency_ms);csv_field(r[i].service);putchar(',');csv_field(r[i].banner);putchar('\n');}}
 else if(nf_output_mode()==NF_OUTPUT_JSON){printf("{\"host\":");nf_output_json_escape(stdout,host);printf(",\"engine\":");nf_output_json_escape(stdout,engine);printf(",\"results\":[");int first=1;for(size_t i=0;i<count;i++){if(open_only&&r[i].status!=1)continue;if(!first)putchar(',');first=0;printf("{\"port\":%u,\"status\":",(unsigned)r[i].port);nf_output_json_escape(stdout,nf_tcp_status_name(r[i].status));printf(",\"latency_ms\":%d,\"service\":",r[i].latency_ms);nf_output_json_escape(stdout,r[i].service);printf(",\"banner\":");nf_output_json_escape(stdout,r[i].banner);putchar('}');}printf("],\"stats\":{\"scanned\":%zu,\"open\":%zu,\"closed\":%zu,\"errors\":%zu,\"avg_ms\":%.2f,\"p50_ms\":%d,\"p95_ms\":%d}}\n",st.total,st.open,st.closed,st.errors,st.avg_ms,st.p50_ms,st.p95_ms);}
 else{printf("\nNetForge TCP Scan — %s [%s]\n\n",host,engine);printf("%-8s %-12s %-8s %-16s %s\n","PORT","STATUS","LATENCY","SERVICE","BANNER");puts("--------------------------------------------------------------------------");for(size_t i=0;i<count;i++){if(open_only&&r[i].status!=1)continue;printf("%-8u %-12s %-8d %-16s %s\n",(unsigned)r[i].port,nf_tcp_status_name(r[i].status),r[i].latency_ms,r[i].service,r[i].banner[0]?r[i].banner:"-");}printf("\nScanned: %zu | Open: %zu | Closed: %zu | Errors: %zu\nLatency ms: min=%d avg=%.2f p50=%d p95=%d max=%d\n",st.total,st.open,st.closed,st.errors,st.min_ms,st.avg_ms,st.p50_ms,st.p95_ms,st.max_ms);}
 free(r);free(ports);return NF_OK;
}
