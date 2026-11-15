#define _POSIX_C_SOURCE 200809L
#include "tshlib.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int number(const char *s,unsigned long max,unsigned long *value)
{
    char *end;
    if(!s || !*s || *s<'0' || *s>'9') return -1;
    errno=0;*value=strtoul(s,&end,10);
    return errno || *end || !*value || *value>max ? -1:0;
}
static int usage(const char *name)
{
    fprintf(stderr,"Usage: %s shell \"command\" [--bg]\n"
      "       %s ps\n       %s kill PID\n       %s killall\n"
      "Environment: TSHPORT required; TSHHOST defaults to 127.0.0.1\n",
      name,name,name,name);
    return 2;
}
int main(int argc,char **argv)
{
    if(argc<2)return usage(argv[0]);
    const char *host=getenv("TSHHOST");if(!host || !*host)host="127.0.0.1";
    unsigned long port;
    if(number(getenv("TSHPORT"),65535,&port)<0) {
        fprintf(stderr,"Set TSHPORT to a number between 1 and 65535.\n");return 2;
    }
    if(strcmp(argv[1],"shell")==0) {
        if(argc!=3 && !(argc==4 && strcmp(argv[3],"--bg")==0))return usage(argv[0]);
        tsh_job_result result;
        if(tsh_shell(host,(uint16_t)port,argv[2],argc==4,&result)<0) {
            perror("tsh_shell");return 1;
        }
        printf("Job PID: %u\n",result.pid);
        if(result.completed) {
            printf("Completed. Exit status: %u\n",result.exit_code);
            return (int)(result.exit_code & 255U);
        }
        puts("Background job accepted.");return 0;
    }
    if(strcmp(argv[1],"ps")==0 && argc==2) {
        tsh_job_info jobs[64];size_t count;
        if(tsh_ps(host,(uint16_t)port,jobs,64,&count)<0) {perror("tsh_ps");return 1;}
        printf("Active jobs: %zu\nPID       MODE  STATE       COMMAND\n",count);
        for(size_t i=0;i<count;i++)
            printf("%-9u %-5s %-11s %s\n",jobs[i].pid,jobs[i].background?"BG":"FG",
                jobs[i].stopping?"STOPPING":"RUNNING",jobs[i].command);
        return 0;
    }
    if(strcmp(argv[1],"kill")==0 && argc==3) {
        unsigned long pid;
        if(number(argv[2],2147483647UL,&pid)<0) return usage(argv[0]);
        if(tsh_kill(host,(uint16_t)port,(uint32_t)pid)<0) {perror("tsh_kill");return 1;}
        printf("SIGKILL sent to tracked job %lu.\n",pid);return 0;
    }
    if(strcmp(argv[1],"killall")==0 && argc==2) {
        uint32_t count;
        if(tsh_killall(host,(uint16_t)port,&count)<0) {
            perror("tsh_killall");fprintf(stderr,"Signalled jobs: %u\n",count);return 1;
        }
        printf("SIGKILL sent to %u tracked job group(s).\n",count);return 0;
    }
    return usage(argv[0]);
}
