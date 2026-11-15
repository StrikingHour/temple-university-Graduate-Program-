#define _XOPEN_SOURCE 700
#include "tshlib.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#define MAX_WORKERS 64
static int positive(const char *s,unsigned long limit,unsigned long *value)
{
    char *end;
    if(!s || !*s || *s<'0' || *s>'9')return -1;
    errno=0;*value=strtoul(s,&end,10);
    return errno || *end || !*value || *value>limit ? -1:0;
}
/* The current myshell tokenizes on whitespace, without shell quoting.
 * Reject ambiguous tokens instead of silently changing an argument.
 */
static int safe_token(const char *s)
{
    return *s && !strpbrk(s," \t\r\n\a|><&;\"'\\");
}
static void rollback(const char *host,unsigned short port,uint32_t *pids,size_t count)
{
    for(size_t i=0;i<count;i++) {
        if(tsh_kill(host,port,pids[i])<0 && errno!=ESRCH)
            fprintf(stderr,"Rollback failed for job %u: %s\n",pids[i],strerror(errno));
    }
}
int main(int argc,char **argv)
{
    if(argc<3) {
        fprintf(stderr,"Usage: %s PROGRAM P [program-argument ...]\n",argv[0]);
        return 2;
    }
    unsigned long workers,port;
    if(positive(argv[2],MAX_WORKERS,&workers)<0) {
        fprintf(stderr,"P must be an integer from 1 to %d.\n",MAX_WORKERS);return 2;
    }
    if(positive(getenv("TSHPORT"),65535,&port)<0) {
        fprintf(stderr,"Set TSHPORT to your daemon port (1..65535).\n");return 2;
    }
    const char *host=getenv("TSHHOST");if(!host || !*host)host="127.0.0.1";
    /* Resolve from the launcher's current directory, not daemon cwd or PATH.
     * This lab runs both programs on the same tec2 filesystem.
     */
    char *path=realpath(argv[1],NULL);
    if(!path) {perror("program path");return 1;}
    struct stat st;
    if(stat(path,&st)<0 || !S_ISREG(st.st_mode) || access(path,X_OK)<0) {
        fprintf(stderr,"Program must be an executable regular file: %s\n",path);
        free(path);return 1;
    }
    char command[TSH_COMMAND_SIZE];
    if(!safe_token(path) || strlen(path)>=sizeof command) {
        fprintf(stderr,"Executable path is too long or unsupported by myshell.\n");
        free(path);return 2;
    }
    strcpy(command,path);free(path);
    for(int i=3;i<argc;i++) {
        if(!safe_token(argv[i]) || strlen(command)+1+strlen(argv[i])>=sizeof command) {
            fprintf(stderr,"Argument is empty, contains shell syntax/whitespace, or command is too long.\n");
            return 2;
        }
        strcat(command," ");strcat(command,argv[i]);
    }
    uint32_t pids[MAX_WORKERS];size_t started=0;
    printf("Daemon: %s:%lu\nCommand: %s\nRequested workers: %lu\n",host,port,command,workers);
    fflush(stdout);
    for(unsigned long i=0;i<workers;i++) {
        tsh_job_result result;
        if(tsh_shell(host,(unsigned short)port,command,1,&result)<0) {
            fprintf(stderr,"Launch request %lu failed: %s\n",i+1,strerror(errno));
            fprintf(stderr,"Stopping the %zu jobs already accepted by this invocation.\n",started);
            rollback(host,(unsigned short)port,pids,started);
            return 1;
        }
        pids[started++]=result.pid;
        printf("Worker %lu: job PID=%u\n",i+1,result.pid);
        fflush(stdout);
    }
    printf("Accepted %zu background jobs. Check PS and daemon output for execution/completion.\n",started);
    return 0;
}
