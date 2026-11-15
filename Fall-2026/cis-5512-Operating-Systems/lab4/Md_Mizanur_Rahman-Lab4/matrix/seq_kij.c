/* Lab4 adaptation of the student's Lab1 k-i-j multiplication.
   Same deterministic inputs and double storage as lab4 matrix/client.c.
   No threads, sockets, BLAS or OpenMP. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <math.h>
static double now(void) {
    struct timespec t;
    if(clock_gettime(CLOCK_MONOTONIC,&t)) {perror("clock_gettime");exit(1);}
    return (double)t.tv_sec+(double)t.tv_nsec/1e9;
}
static uint32_t state=123456789;
static double random_value(void) {state=1664525u*state+1013904223u;return (double)(state%11u);}
int main(int argc,char **argv) {
    if(argc!=2) {fprintf(stderr,"Usage: %s N (1..5000)\n",argv[0]);return 2;}
    char *end;errno=0;long parsed=strtol(argv[1],&end,10);
    if(errno||!*argv[1]||*end||parsed<1||parsed>5000) {fprintf(stderr,"N must be an integer from 1 to 5000\n");return 2;}
    unsigned n=(unsigned)parsed;size_t count=(size_t)n*n;
    double *a=malloc(count*sizeof(*a)),*b=malloc(count*sizeof(*b)),*c=malloc(count*sizeof(*c));
    if(!a||!b||!c) {perror("malloc");free(a);free(b);free(c);return 1;}
    /* Initialize and touch every page before timing, as in the original Lab1. */
    for(size_t t=0;t<count;t++) {a[t]=random_value();b[t]=random_value();c[t]=0;}
    double start=now();
#ifdef BASELINE_IKJ
    for(unsigned i=0;i<n;i++)for(unsigned k=0;k<n;k++) {
#else
    for(unsigned k=0;k<n;k++)for(unsigned i=0;i<n;i++) {
#endif
        double x=a[(size_t)i*n+k];
        for(unsigned j=0;j<n;j++)c[(size_t)i*n+j]+=x*b[(size_t)k*n+j];
    }
    double elapsed=now()-start;
    /* Consume ALL results after timing; avoid dead-result benchmark artifacts. */
    double checksum=0;int finite=1;
    for(size_t t=0;t<count;t++) {if(!isfinite(c[t]))finite=0;checksum+=c[t];}
    size_t checks=n<=256?count:100;double max_error=0;
    for(size_t t=0;t<checks;t++) {
        unsigned i=n<=256?(unsigned)(t/n):(unsigned)((t*7919u)%n);
        unsigned j=n<=256?(unsigned)(t%n):(unsigned)((t*104729u+17u)%n);
        double expected=0;
        for(unsigned k=0;k<n;k++)expected+=a[(size_t)i*n+k]*b[(size_t)k*n+j];
        double error=fabs(c[(size_t)i*n+j]-expected);
        if(error>max_error)max_error=error;
    }
#ifdef BASELINE_IKJ
    const char *order="ikj";
#else
    const char *order="kij";
#endif
    int pass=finite&&max_error==0;
    printf("N=%u order=%s sequential_seconds=%.6f\n",n,order,elapsed);
    printf("checksum=%.0f\n",checksum);
    printf("Verification: %s (%s, %zu cells, max_abs_error=%.3g)\n",pass?"PASS":"FAIL",n<=256?"full":"sampled",checks,max_error);
    free(a);free(b);free(c);return pass?0:1;
}
