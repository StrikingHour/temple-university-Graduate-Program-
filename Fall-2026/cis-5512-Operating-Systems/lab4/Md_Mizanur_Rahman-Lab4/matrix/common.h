#ifndef MATRIX_COMMON_H
#define MATRIX_COMMON_H
#define _POSIX_C_SOURCE 200809L
#include "tuplelib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
/* Native double / uint32_t payloads: same-machine lab only. */
typedef struct {uint32_t n,g,p;} Config;
typedef struct {uint32_t row,rows; double values[];} Stripe;
typedef struct {uint32_t pid,tasks; double seconds;} Done;
static inline void die(const char *s) {perror(s);exit(1);}
static inline double now(void) {struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))die("clock");return t.tv_sec+t.tv_nsec*1e-9;}
static inline void *alloc(size_t n) {void *p=calloc(1,n);if(!p)die("allocation");return p;}
static inline unsigned number(const char *s,unsigned max) {char *end;long n=strtol(s,&end,10);if(!*s || *end || n<1 || n>max) {fprintf(stderr,"Invalid number: %s (range 1..%u)\n",s,max);exit(2);}return (unsigned)n;}
static inline void check_run(const char *s) {size_t n=strlen(s);if(!n || n>40)exit(2);for(size_t i=0;i<n;i++)if(!((s[i]>='a'&&s[i]<='z')||(s[i]>='A'&&s[i]<='Z')||(s[i]>='0'&&s[i]<='9')||s[i]=='_')) {fprintf(stderr,"Run ID: use only letters, digits, underscore\n");exit(2);}}
static inline void key(char *dst,const char *run,const char *suffix) {snprintf(dst,128,"mx_%s_%s",run,suffix);}
static inline void put(const char *k,const void *p,size_t n) {if(tuple_put(k,p,n))die("tuple_put");}
static inline void *fetch(const char *pattern,int take,size_t *n) {void *p;if(tuple_fetch(pattern,take,&p,n))die("tuple_fetch");return p;}
static inline void *exact(const char *k,int take,size_t expected) {char pat[132];snprintf(pat,sizeof(pat),"^%s$",k);size_t n;void *p=fetch(pat,take,&n);if(n!=expected) {fprintf(stderr,"Invalid payload length for %s\n",k);exit(1);}return p;}
static inline void multiply(const double *a,const double *b,double *c,unsigned rows,unsigned n) {
    for(unsigned i=0;i<rows;i++)for(unsigned k=0;k<n;k++) {double x=a[(size_t)i*n+k];for(unsigned j=0;j<n;j++)c[(size_t)i*n+j]+=x*b[(size_t)k*n+j];}
}
#endif
