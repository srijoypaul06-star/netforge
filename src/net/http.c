#define _POSIX_C_SOURCE 200112L
#include "netforge/http.h"
#include "netforge/tcp.h"
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

static long long ms_now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (long long)t.tv_sec*1000LL+t.tv_nsec/1000000LL;}
int nf_http_probe(const char *host,int port,int timeout_ms,nf_http_result_t *r){
    memset(r,0,sizeof(*r));r->status_code=-1;
    struct addrinfo hints={0},*ai=NULL;hints.ai_socktype=SOCK_STREAM;hints.ai_family=AF_UNSPEC;
    char svc[16];snprintf(svc,sizeof(svc),"%d",port);if(getaddrinfo(host,svc,&hints,&ai)!=0)return -1;
    int fd=-1;long long start=ms_now();
    for(struct addrinfo*p=ai;p;p=p->ai_next){fd=socket(p->ai_family,p->ai_socktype,p->ai_protocol);if(fd<0)continue;int fl=fcntl(fd,F_GETFL,0);fcntl(fd,F_SETFL,fl|O_NONBLOCK);if(connect(fd,p->ai_addr,p->ai_addrlen)==0)break;if(errno==EINPROGRESS){struct pollfd pf={fd,POLLOUT,0};if(poll(&pf,1,timeout_ms)>0){int e=0;socklen_t l=sizeof(e);getsockopt(fd,SOL_SOCKET,SO_ERROR,&e,&l);if(e==0)break;}}close(fd);fd=-1;}
    freeaddrinfo(ai);if(fd<0)return -1;
    fcntl(fd,F_SETFL,fcntl(fd,F_GETFL,0)&~O_NONBLOCK);
    struct timeval tv={timeout_ms/1000,(timeout_ms%1000)*1000};setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv));
    char req[512];snprintf(req,sizeof(req),"HEAD / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\nUser-Agent: NetForge/%s\r\n\r\n",host,"1.0.0");
    if(send(fd,req,strlen(req),0)<0){close(fd);return -1;}
    char buf[8192];ssize_t n=recv(fd,buf,sizeof(buf)-1U,0);close(fd);if(n<=0)return -1;buf[n]='\0';r->latency_ms=(int)(ms_now()-start);
    sscanf(buf,"HTTP/%*s %d",&r->status_code);
    char *line=strstr(buf,"\r\nServer:");if(line)sscanf(line,"\r\nServer: %127[^\r\n]",r->server);
    line=strstr(buf,"\r\nContent-Type:");if(line)sscanf(line,"\r\nContent-Type: %127[^\r\n]",r->content_type);
    line=strstr(buf,"\r\nContent-Length:");if(line)sscanf(line,"\r\nContent-Length: %ld",&r->content_length);
    strncpy(r->final_host,host,sizeof(r->final_host)-1U);return 0;
}
