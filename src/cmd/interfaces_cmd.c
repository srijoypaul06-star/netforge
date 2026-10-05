#include "netforge/app.h"
#include "netforge/interfaces.h"
#include <stdio.h>
int nf_cmd_interfaces(void){nf_interface_t*r=NULL;size_t n=0;if(nf_interfaces_list(&r,&n))return NF_ERR_NETWORK;printf("Network interfaces\n\n%-16s %-40s %-40s\n","INTERFACE","ADDRESS","NETMASK");printf("------------------------------------------------------------------------------------------------\n");for(size_t i=0;i<n;i++)printf("%-16s %-40s %-40s\n",r[i].name,r[i].address,r[i].netmask);nf_interfaces_free(r);return 0;}
