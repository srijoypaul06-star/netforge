#define _GNU_SOURCE
#include "netforge/epoll_scan.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct { int fd; size_t idx; long long start_ms; } conn_t;
static long long mono_ms(void){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return (long long)ts.tv_sec*1000LL+ts.tv_nsec/1000000LL;}
static int make_addr(const struct addrinfo *base,uint16_t port,struct sockaddr_storage *ss,socklen_t *len){
    memcpy(ss,base->ai_addr,base->ai_addrlen);*len=(socklen_t)base->ai_addrlen;
    if(base->ai_family==AF_INET){((struct sockaddr_in*)ss)->sin_port=htons(port);}
    else if(base->ai_family==AF_INET6){((struct sockaddr_in6*)ss)->sin6_port=htons(port);}
    else{return -1;}return 0;
}
static int start_one(int ep,const struct addrinfo *base,const uint16_t *ports,size_t idx,nf_tcp_result_t *r,conn_t *c){
    r[idx].port=ports[idx];snprintf(r[idx].service,sizeof(r[idx].service),"%s",nf_tcp_guess_service(ports[idx]));r[idx].status=0;
    int fd=socket(base->ai_family,SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC,base->ai_protocol);if(fd<0){r[idx].status=-1;return -1;}
    struct sockaddr_storage ss;socklen_t sl;if(make_addr(base,ports[idx],&ss,&sl)!=0){close(fd);r[idx].status=-1;return -1;}
    long long st=mono_ms();int rc=connect(fd,(struct sockaddr*)&ss,sl);
    if(rc==0){r[idx].status=1;r[idx].latency_ms=(int)(mono_ms()-st);close(fd);return 1;}
    if(errno!=EINPROGRESS){r[idx].latency_ms=(int)(mono_ms()-st);close(fd);return 1;}
    c->fd=fd;c->idx=idx;c->start_ms=st;struct epoll_event ev={.events=EPOLLOUT|EPOLLERR|EPOLLHUP,.data.ptr=c};
    if(epoll_ctl(ep,EPOLL_CTL_ADD,fd,&ev)!=0){close(fd);c->fd=-1;r[idx].status=-1;return -1;}return 0;
}
int nf_epoll_scan(const char *host,const uint16_t *ports,size_t count,int timeout_ms,unsigned max_inflight,nf_tcp_result_t *results){
    if(!host||!ports||!results||!count||timeout_ms<=0)return NF_ERR_USAGE;
    if(max_inflight==0)max_inflight=64;
    if(max_inflight>4096)max_inflight=4096;
    memset(results,0,count*sizeof(*results));struct addrinfo hints={0},*res=NULL;hints.ai_socktype=SOCK_STREAM;hints.ai_family=AF_UNSPEC;
    if(getaddrinfo(host,NULL,&hints,&res)!=0)return NF_ERR_NETWORK;
    struct addrinfo *base=res;while(base&&base->ai_family!=AF_INET&&base->ai_family!=AF_INET6)base=base->ai_next;
    if(!base){freeaddrinfo(res);return NF_ERR_NETWORK;}
    int ep=epoll_create1(EPOLL_CLOEXEC);if(ep<0){freeaddrinfo(res);return NF_ERR_NETWORK;}
    conn_t *conns=calloc(count,sizeof(*conns));if(!conns){close(ep);freeaddrinfo(res);return NF_ERR_MEMORY;}
    for(size_t i=0;i<count;i++)conns[i].fd=-1;
    size_t next=0,done=0,inflight=0;struct epoll_event events[256];int fatal=0;
    while(done<count&&!fatal){
        while(next<count&&inflight<max_inflight){int s=start_one(ep,base,ports,next,results,&conns[next]);next++;if(s==0)inflight++;else done++;}
        if(inflight==0)continue;
        int ne=epoll_wait(ep,events,256,timeout_ms);long long now=mono_ms();
        if(ne<0){if(errno==EINTR)continue;fatal=1;break;}
        for(int e=0;e<ne;e++){conn_t*c=events[e].data.ptr;if(c->fd<0)continue;int err=0;socklen_t el=sizeof(err);(void)getsockopt(c->fd,SOL_SOCKET,SO_ERROR,&err,&el);results[c->idx].status=(err==0)?1:0;results[c->idx].latency_ms=(int)(now-c->start_ms);(void)epoll_ctl(ep,EPOLL_CTL_DEL,c->fd,NULL);close(c->fd);c->fd=-1;inflight--;done++;}
        for(size_t i=0;i<next;i++){conn_t*c=&conns[i];if(c->fd>=0&&now-c->start_ms>=timeout_ms){results[c->idx].status=0;results[c->idx].latency_ms=timeout_ms;(void)epoll_ctl(ep,EPOLL_CTL_DEL,c->fd,NULL);close(c->fd);c->fd=-1;inflight--;done++;}}
    }
    for(size_t i=0;i<count;i++){if(conns[i].fd>=0)close(conns[i].fd);}free(conns);close(ep);freeaddrinfo(res);return fatal?NF_ERR_NETWORK:NF_OK;
}
