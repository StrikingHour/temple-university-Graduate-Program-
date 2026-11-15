#include "common.h"
static uint32_t state=123456789;
static double random_value(void) {state=1664525u*state+1013904223u;return (double)(state%11u);}
int main(int argc,char **argv) {
    if(argc!=5) {fprintf(stderr,"Usage: client N G P RUN_ID\n");return 2;}
    unsigned n=number(argv[1],5000),g=number(argv[2],n),p=number(argv[3],64);const char *run=argv[4];check_run(run);
    size_t bytes=(size_t)n*n*sizeof(double);double *a=alloc(bytes),*b=alloc(bytes),*c=alloc(bytes);
    for(size_t i=0;i<(size_t)n*n;i++) {a[i]=random_value();b[i]=random_value();}
    char k[128],pattern[128];Config cfg={n,g,p};double start=now();
    key(k,run,"B");put(k,b,bytes);key(k,run,"config");put(k,&cfg,sizeof(cfg));
    unsigned tasks=(n+g-1)/g;
    for(unsigned row=0;row<n;row+=g) {
        unsigned rows=n-row<g?n-row:g;size_t len=sizeof(Stripe)+(size_t)rows*n*sizeof(double);Stripe *s=alloc(len);s->row=row;s->rows=rows;
        memcpy(s->values,a+(size_t)row*n,len-sizeof(Stripe));snprintf(k,sizeof(k),"mx_%s_task_%08u",run,row);put(k,s,len);free(s);
    }
    unsigned char *seen=alloc(tasks);snprintf(pattern,sizeof(pattern),"^mx_%s_result_.*$",run);
    for(unsigned t=0;t<tasks;t++) {
        size_t len;Stripe *r=fetch(pattern,1,&len);
        if(len<sizeof(Stripe)||r->row>=n||r->row%g||r->rows!=(n-r->row<g?n-r->row:g)||len!=sizeof(Stripe)+(size_t)r->rows*n*sizeof(double)||seen[r->row/g]) {fprintf(stderr,"Invalid or duplicate result\n");return 1;}
        seen[r->row/g]=1;memcpy(c+(size_t)r->row*n,r->values,len-sizeof(Stripe));free(r);
    }
    double elapsed=now()-start;
    for(unsigned i=0;i<p;i++) {Stripe stop={0};snprintf(k,sizeof(k),"mx_%s_task_stop_%08u",run,i);put(k,&stop,sizeof(stop));}
    snprintf(pattern,sizeof(pattern),"^mx_%s_done_.*$",run);unsigned total=0;
    for(unsigned i=0;i<p;i++) {size_t len;Done *d=fetch(pattern,1,&len);if(len!=sizeof(Done))return 1;total+=d->tasks;printf("worker PID=%u stripes=%u compute=%.6f s\n",d->pid,d->tasks,d->seconds);free(d);}
    if(total!=tasks)return 1;
    /* Independent i-j-k reference; all cells for small N, deterministic samples for large N. */
    size_t checks=n<=256?(size_t)n*n:100;double max_error=0;
    for(size_t t=0;t<checks;t++) {
        unsigned i=n<=256?(unsigned)(t/n):(unsigned)((t*7919u)%n);
        unsigned j=n<=256?(unsigned)(t%n):(unsigned)((t*104729u+17u)%n);
        double expected=0;for(unsigned z=0;z<n;z++)expected+=a[(size_t)i*n+z]*b[(size_t)z*n+j];
        double error=fabs(c[(size_t)i*n+j]-expected);if(!isfinite(c[(size_t)i*n+j]))error=INFINITY;if(error>max_error)max_error=error;
    }
    printf("N=%u G=%u P=%u stripes=%u parallel_seconds=%.6f\n",n,g,p,tasks,elapsed);
    printf("Verification: %s (%s, %zu cells, max_abs_error=%.3g)\n",max_error==0?"PASS":"FAIL",n<=256?"full":"sampled",checks,max_error);
    free(a);free(b);free(c);free(seen);return max_error==0?0:1;
}
