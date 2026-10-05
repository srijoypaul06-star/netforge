#include "netforge/app.h"
#include "netforge/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int nf_cmd_scan(int,char**,const nf_config_t*);
int nf_cmd_discover(int,char**,const nf_config_t*);
int nf_cmd_probe(int,char**,const nf_config_t*);
int nf_cmd_dns(int,char**);
int nf_cmd_http(int,char**,const nf_config_t*);
int nf_cmd_interfaces(void);
int nf_cmd_inspect(int,char**,const nf_config_t*);
int nf_cmd_tls(int,char**,const nf_config_t*);

void nf_app_usage(const char *p){
    printf("NetForge %s — Linux network diagnostics toolkit\n\n",NF_VERSION);
    printf("Usage:\n  %s [global-options] <command> [command-options]\n\n",p);
    puts("Commands:\n"
         "  scan        TCP scan using pthread or Linux epoll engine\n"
         "  discover    Discover responsive hosts in an IPv4 CIDR range\n"
         "  probe       Probe one TCP endpoint\n"
         "  dns         Resolve A/AAAA records\n"
         "  http        Inspect an HTTP endpoint\n"
         "  tls         Inspect TLS protocol, cipher, and certificate\n"
         "  interfaces  List local network interfaces\n"
         "  inspect     Combined host intelligence report\n");
    puts("Global options:\n"
         "  --timeout MS    Connection timeout (default: 1000)\n"
         "  --workers N     Worker/in-flight concurrency (default: 16)\n"
         "  --json          Machine-readable output where supported\n"
         "  -v              Verbose logging\n"
         "  -h, --help      Show help\n");
    printf("Examples:\n"
           "  %s scan localhost 22,80,443\n"
           "  %s scan localhost 1-1024 --engine epoll --open-only\n"
           "  %s scan localhost 1-1024 --engine epoll --csv\n"
           "  %s discover 192.168.1.0/24 443\n"
           "  %s dns example.com\n"
           "  %s http example.com 80\n"
           "  %s tls example.com 443\n"
           "  %s inspect example.com\n",p,p,p,p,p,p,p,p);
}

int nf_app_run(int argc,char **argv,const nf_config_t *base){
    if(argc<2){nf_app_usage(argv[0]);return 0;}
    nf_config_t cfg=*base;int arg=1;
    for(;arg<argc;arg++){
        if(strcmp(argv[arg],"--timeout")==0&&arg+1<argc){cfg.timeout_ms=atoi(argv[++arg]);continue;}
        if(strcmp(argv[arg],"--workers")==0&&arg+1<argc){cfg.workers=(unsigned)atoi(argv[++arg]);continue;}
        if(strcmp(argv[arg],"-v")==0){cfg.verbose=1;nf_log_set_level(NF_LOG_DEBUG);continue;}
        if(strcmp(argv[arg],"--json")==0){nf_output_set_mode(NF_OUTPUT_JSON);continue;}
        break;
    }
    if(arg>=argc||strcmp(argv[arg],"-h")==0||strcmp(argv[arg],"--help")==0){nf_app_usage(argv[0]);return 0;}
    char **sub=&argv[arg];int n=argc-arg;const char *cmd=sub[0];
    if(strcmp(cmd,"scan")==0)return nf_cmd_scan(n,sub,&cfg);
    if(strcmp(cmd,"discover")==0)return nf_cmd_discover(n,sub,&cfg);
    if(strcmp(cmd,"probe")==0)return nf_cmd_probe(n,sub,&cfg);
    if(strcmp(cmd,"dns")==0)return nf_cmd_dns(n,sub);
    if(strcmp(cmd,"http")==0)return nf_cmd_http(n,sub,&cfg);
    if(strcmp(cmd,"tls")==0)return nf_cmd_tls(n,sub,&cfg);
    if(strcmp(cmd,"interfaces")==0)return nf_cmd_interfaces();
    if(strcmp(cmd,"inspect")==0)return nf_cmd_inspect(n,sub,&cfg);
    fprintf(stderr,"unknown command: %s\n",cmd);return NF_ERR_USAGE;
}
