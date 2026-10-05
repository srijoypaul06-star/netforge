#define _POSIX_C_SOURCE 200112L
#include "netforge/interfaces.h"
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>

int nf_interfaces_list(nf_interface_t **items, size_t *count) {
    *items=NULL; *count=0; struct ifaddrs *ifa=NULL;
    if (getifaddrs(&ifa) != 0) return -1;
    for (struct ifaddrs *p=ifa; p; p=p->ifa_next) {
        if (!p->ifa_addr) continue;
        int family=p->ifa_addr->sa_family; if (family != AF_INET && family != AF_INET6) continue;
        nf_interface_t *tmp=realloc(*items, (*count+1U)*sizeof(**items)); if(!tmp){freeifaddrs(ifa); nf_interfaces_free(*items);*items=NULL;*count=0;return -1;}
        *items=tmp; nf_interface_t *x=&(*items)[*count]; memset(x,0,sizeof(*x));
        strncpy(x->name,p->ifa_name,sizeof(x->name)-1U); x->flags=p->ifa_flags;
        getnameinfo(p->ifa_addr, family==AF_INET?sizeof(struct sockaddr_in):sizeof(struct sockaddr_in6), x->address,sizeof(x->address),NULL,0,NI_NUMERICHOST);
        if(p->ifa_netmask) getnameinfo(p->ifa_netmask, family==AF_INET?sizeof(struct sockaddr_in):sizeof(struct sockaddr_in6),x->netmask,sizeof(x->netmask),NULL,0,NI_NUMERICHOST);
        (*count)++;
    }
    freeifaddrs(ifa); return 0;
}
void nf_interfaces_free(nf_interface_t *items){free(items);}
