#include "netforge/app.h"
#include "netforge/tcp.h"
#include <stdio.h>
#include <stdlib.h>
int nf_cmd_probe(int argc,char**argv,const nf_config_t*cfg){if(argc<3){fprintf(stderr,"usage: netforge probe <host> <port>\n");return NF_ERR_USAGE;}char*e=NULL;long p=strtol(argv[2],&e,10);if(!*argv[2]||*e||p<1||p>65535){fprintf(stderr,"invalid port\n");return NF_ERR_USAGE;}nf_tcp_result_t r;nf_tcp_probe(argv[1],(uint16_t)p,cfg->timeout_ms,&r);printf("%s:%ld -> %s (%d ms)\n",argv[1],p,nf_tcp_status_name(r.status),r.latency_ms);return 0;}
