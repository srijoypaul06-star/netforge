#include "netforge/stats.h"
#include <stdlib.h>
#include <string.h>
static int cmp_int(const void *a,const void *b){int x=*(const int*)a,y=*(const int*)b;return (x>y)-(x<y);}
void nf_scan_stats(const nf_tcp_result_t *r,size_t n,nf_scan_stats_t *o){
 memset(o,0,sizeof(*o)); o->total=n; if(!n)return; int *v=calloc(n,sizeof(*v)); if(!v)return; size_t m=0; long long sum=0;
 for(size_t i=0;i<n;i++){if(r[i].status==1)o->open++;else if(r[i].status==0)o->closed++;else o->errors++; if(r[i].latency_ms>=0){v[m++]=r[i].latency_ms;sum+=r[i].latency_ms;}}
 if(m){qsort(v,m,sizeof(*v),cmp_int);o->min_ms=v[0];o->max_ms=v[m-1];o->avg_ms=(double)sum/(double)m;o->p50_ms=v[(m-1)*50/100];o->p95_ms=v[(m-1)*95/100];} free(v);
}
