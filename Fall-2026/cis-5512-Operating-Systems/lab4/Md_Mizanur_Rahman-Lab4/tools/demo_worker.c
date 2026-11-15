#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
int main(int argc,char **argv)
{
    long seconds=60;char *end;
    if(argc>2) {fprintf(stderr,"Usage: %s [seconds: 1..3600]\n",argv[0]);return 2;}
    if(argc==2) {
        errno=0;seconds=strtol(argv[1],&end,10);
        if(errno || end==argv[1] || *end || seconds<1 || seconds>3600) {
            fprintf(stderr,"Seconds must be 1..3600.\n");return 2;
        }
    }
    printf("demo_worker START pid=%ld parent=%ld duration=%ld seconds\n",
        (long)getpid(),(long)getppid(),seconds);fflush(stdout);
    struct timespec remaining={seconds,0};
    while(nanosleep(&remaining,&remaining)<0) {
        if(errno!=EINTR) {perror("nanosleep");return 1;}
    }
    printf("demo_worker DONE pid=%ld\n",(long)getpid());fflush(stdout);
    return 0;
}
