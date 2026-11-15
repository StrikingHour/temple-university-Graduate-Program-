#include "common.h"
int main(int argc,char **argv) {
    if(argc!=2) {fprintf(stderr,"Usage: worker RUN_ID\n");return 2;}check_run(argv[1]);
    char k[128],pattern[128];key(k,argv[1],"config");Config *cfg=exact(k,0,sizeof(Config));
    unsigned n=cfg->n,g=cfg->g;if(!n||n>5000||!g||g>n||!cfg->p||cfg->p>64)return 1;
    key(k,argv[1],"B");double *b=exact(k,0,(size_t)n*n*sizeof(double));
    snprintf(pattern,sizeof(pattern),"^mx_%s_task_.*$",argv[1]);Done done={.pid=(uint32_t)getpid()};
    for(;;) {
        size_t bytes;Stripe *s=fetch(pattern,1,&bytes);
        if(bytes<sizeof(Stripe))return 1;
        if(!s->rows) {free(s);break;}
        if(s->rows>g || s->row>=n || s->rows>n-s->row || bytes!=sizeof(Stripe)+(size_t)s->rows*n*sizeof(double))return 1;
        Stripe *r=alloc(bytes);r->row=s->row;r->rows=s->rows;
        double t=now();multiply(s->values,b,r->values,s->rows,n);done.seconds+=now()-t;done.tasks++;
        snprintf(k,sizeof(k),"mx_%s_result_%08u",argv[1],r->row);put(k,r,bytes);free(r);free(s);
    }
    snprintf(k,sizeof(k),"mx_%s_done_%u",argv[1],done.pid);put(k,&done,sizeof(done));
    printf("worker %u finished: %u stripes, compute %.6f s\n",done.pid,done.tasks,done.seconds);
    free(b);free(cfg);return 0;
}
