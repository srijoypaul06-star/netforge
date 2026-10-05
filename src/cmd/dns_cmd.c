#include "netforge/app.h"
#include "netforge/dns.h"
#include "netforge/output.h"
#include <stdio.h>
#include <string.h>
int nf_cmd_dns(int argc,char**argv){if(argc<2){fprintf(stderr,"usage: netforge dns <host>\n");return NF_ERR_USAGE;}nf_dns_record_t*r=NULL;size_t n=0;int rc=nf_dns_resolve(argv[1],&r,&n);if(rc){fprintf(stderr,"DNS resolution failed for %s\n",argv[1]);return rc;}if(nf_output_mode()==NF_OUTPUT_JSON){printf("{\"host\":");nf_output_json_escape(stdout,argv[1]);printf(",\"records\":[");for(size_t i=0;i<n;i++){if(i)putchar(',');printf("{\"type\":");nf_output_json_escape(stdout,nf_dns_family_name(r[i].family));printf(",\"address\":");nf_output_json_escape(stdout,r[i].address);printf("}");}printf("]}\n");}else{printf("DNS records for %s\n\n%-8s %s\n----------------------------\n",argv[1],"TYPE","ADDRESS");for(size_t i=0;i<n;i++)printf("%-8s %s\n",nf_dns_family_name(r[i].family),r[i].address);}nf_dns_free(r);return 0;}
