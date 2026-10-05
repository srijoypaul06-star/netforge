#define _POSIX_C_SOURCE 200809L
#include "netforge/tls.h"
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static long long now_ms(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (long long)t.tv_sec*1000LL+t.tv_nsec/1000000LL;}
static int connect_tcp(const char *host,int port,int timeout_ms){
 struct addrinfo hints={0},*ai=NULL; hints.ai_socktype=SOCK_STREAM; hints.ai_family=AF_UNSPEC;
 char svc[16]; snprintf(svc,sizeof(svc),"%d",port); if(getaddrinfo(host,svc,&hints,&ai)!=0)return -1;
 int fd=-1; for(struct addrinfo*p=ai;p;p=p->ai_next){fd=socket(p->ai_family,p->ai_socktype,p->ai_protocol);if(fd<0)continue;struct timeval tv={timeout_ms/1000,(timeout_ms%1000)*1000};setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv));if(connect(fd,p->ai_addr,p->ai_addrlen)==0)break;close(fd);fd=-1;} freeaddrinfo(ai);return fd;
}
static void asn1_time_text(const ASN1_TIME *t,char *out,size_t cap){BIO*b=BIO_new(BIO_s_mem());if(!b)return;if(ASN1_TIME_print(b,t)==1){int n=BIO_read(b,out,(int)cap-1);if(n>0)out[n]='\0';}BIO_free(b);}
int nf_tls_probe(const char *host,int port,int timeout_ms,nf_tls_result_t*out){
 if(!host||!out||port<1||port>65535||timeout_ms<=0) return -1;
 memset(out,0,sizeof(*out));
 long long start=now_ms();
 int fd=connect_tcp(host,port,timeout_ms);
 if(fd<0) return -1;
 SSL_CTX*ctx=SSL_CTX_new(TLS_client_method());if(!ctx){close(fd);return -1;}SSL_CTX_set_default_verify_paths(ctx);SSL_CTX_set_verify(ctx,SSL_VERIFY_PEER,NULL);
 SSL*ssl=SSL_new(ctx);if(!ssl){SSL_CTX_free(ctx);close(fd);return -1;}SSL_set_fd(ssl,fd);SSL_set_tlsext_host_name(ssl,host);SSL_set1_host(ssl,host);
 int rc=SSL_connect(ssl);out->latency_ms=(int)(now_ms()-start);if(rc==1){out->ok=1;snprintf(out->protocol,sizeof(out->protocol),"%s",SSL_get_version(ssl));snprintf(out->cipher,sizeof(out->cipher),"%s",SSL_get_cipher_name(ssl));out->verify_result=SSL_get_verify_result(ssl);X509*cert=SSL_get1_peer_certificate(ssl);if(cert){X509_NAME_oneline(X509_get_subject_name(cert),out->subject,(int)sizeof(out->subject));X509_NAME_oneline(X509_get_issuer_name(cert),out->issuer,(int)sizeof(out->issuer));asn1_time_text(X509_get0_notBefore(cert),out->not_before,sizeof(out->not_before));asn1_time_text(X509_get0_notAfter(cert),out->not_after,sizeof(out->not_after));X509_free(cert);}}
 SSL_shutdown(ssl);SSL_free(ssl);SSL_CTX_free(ctx);close(fd);return out->ok?0:-1;
}
