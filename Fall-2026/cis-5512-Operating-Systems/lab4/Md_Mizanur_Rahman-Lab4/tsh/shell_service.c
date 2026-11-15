#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "tsh_shell.h"
#define MAX_JOBS 64
static struct {
    pid_t pid;
    int fd;
    int background;
    int stopping;
    char command[TSH_COMMAND_SIZE];
} jobs[MAX_JOBS];
static char shell_path[4096];
static int transfer(int fd, void *data, size_t size, int sending)
{
    char *p = data;
    while (size) {
        ssize_t n = sending ? send(fd,p,size,MSG_NOSIGNAL) : recv(fd,p,size,0);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return -1;
        p += n; size -= (size_t)n;
    }
    return 0;
}
static void reply(int fd, int accepted, pid_t pid, int done, int code, int err)
{
    tsh_shell_response r = {htonl(accepted),htonl((uint32_t)pid),
        htonl(done),htonl(code),htonl(err)};
    (void)transfer(fd,&r,sizeof r,1);
}
int shell_service_init(void)
{
    const char *p = getenv("MSH_PATH");
    if (!p || p[0] != '/' || strlen(p) >= sizeof shell_path) {
        fprintf(stderr,"Set MSH_PATH to the absolute myshell executable path.\n");
        return -1;
    }
    strcpy(shell_path,p);
    if (access(shell_path,X_OK) < 0) { perror("MSH_PATH"); return -1; }
    for (int i=0;i<MAX_JOBS;i++) jobs[i].fd=-1;
    return atexit(shell_service_shutdown);
}
int shell_service_launch(int fd)
{
    tsh_shell_request r;
    if (transfer(fd,&r,sizeof r,0)<0) return 0;
    if (!memchr(r.command,0,sizeof r.command) || ntohl(r.background)>1) {
        reply(fd,0,0,0,0,EINVAL); return 0;
    }
    size_t n=strlen(r.command);
    while (n && (r.command[n-1]==' ' || r.command[n-1]=='\t')) r.command[--n]=0;
    int bg=(int)ntohl(r.background);
    if (n && r.command[n-1]=='&') {
        bg=1; r.command[--n]=0;
        while(n && (r.command[n-1]==' ' || r.command[n-1]=='\t')) r.command[--n]=0;
    }
    if (!n || strspn(r.command," \t")==n || strchr(r.command,'&')) {
        reply(fd,0,0,0,0,EINVAL); return 0;
    }
    int slot;
    for(slot=0;slot<MAX_JOBS && jobs[slot].pid;slot++);
    if(slot==MAX_JOBS) {reply(fd,0,0,0,0,EAGAIN);return 0;}
    fflush(NULL);
    pid_t pid=fork();
    if(pid<0) {reply(fd,0,0,0,0,errno);return 0;}
    if(pid==0) {
        if(setpgid(0,0)<0) _exit(126);
        signal(SIGTERM,SIG_DFL); signal(SIGINT,SIG_DFL); signal(SIGPIPE,SIG_DFL);
        int nullfd=open("/dev/null",O_RDONLY);
        if(nullfd<0 || dup2(nullfd,STDIN_FILENO)<0) _exit(126);
        if(nullfd>2) close(nullfd);
        /* Do not pass daemon sockets to the shell or its descendants. */
        long limit=sysconf(_SC_OPEN_MAX);
        if(limit<0) limit=65536;
        for(long d=3;d<limit;d++) close((int)d);
        execl(shell_path,shell_path,"-c",r.command,(char *)NULL);
        perror("exec myshell"); _exit(127);
    }
    (void)setpgid(pid,pid); /* child also sets its process group */
    jobs[slot].pid=pid;
    jobs[slot].fd=bg ? -1 : fd;
    jobs[slot].background=bg;
    jobs[slot].stopping=0;
    strcpy(jobs[slot].command,r.command);
    fprintf(stderr,"[shell] job PID=%ld mode=%s command=%s\n",
        (long)pid,bg?"background":"foreground",r.command);
    if(bg) {reply(fd,1,pid,0,0,0);return 0;}
    return 1;
}
void shell_service_reap(void)
{
    for(int i=0;i<MAX_JOBS;i++) {
        if(!jobs[i].pid) continue;
        int status;
        pid_t result=waitpid(jobs[i].pid,&status,WNOHANG);
        if(result<=0) continue;
        int code=WIFEXITED(status)?WEXITSTATUS(status):128+WTERMSIG(status);
        fprintf(stderr,"[shell] job PID=%ld completed exit=%d\n",(long)result,code);
        if(jobs[i].fd>=0) {
            reply(jobs[i].fd,1,result,1,code,0);
            close(jobs[i].fd);
        }
        jobs[i].pid=0; jobs[i].fd=-1;
    }
}
void shell_service_shutdown(void)
{
    for(int i=0;i<MAX_JOBS;i++) if(jobs[i].pid) {
        kill(-jobs[i].pid,SIGKILL);
        if(jobs[i].fd>=0) close(jobs[i].fd);
    }
}

/* Management is limited to active jobs created by this daemon.
 * SIGKILL terminates the entire tracked process group, including pipelines.
 * Reply means the signal was sent; reaping occurs in the normal event loop.
 */
static int control_reply(int fd, int success, unsigned count, int err)
{
    tsh_control_response r={htonl(success),htonl(count),htonl(err)};
    return transfer(fd,&r,sizeof r,1);
}
void shell_service_ps(int fd)
{
    shell_service_reap();
    unsigned count=0;
    for(int i=0;i<MAX_JOBS;i++) if(jobs[i].pid) count++;
    if(control_reply(fd,1,count,0)<0) return;
    for(int i=0;i<MAX_JOBS;i++) if(jobs[i].pid) {
        tsh_ps_entry row={0};
        row.pid=htonl((uint32_t)jobs[i].pid);
        row.background=htonl(jobs[i].background);
        row.stopping=htonl(jobs[i].stopping);
        strcpy(row.command,jobs[i].command);
        if(transfer(fd,&row,sizeof row,1)<0) return;
    }
}
void shell_service_kill(int fd)
{
    tsh_kill_request r;
    if(transfer(fd,&r,sizeof r,0)<0) return;
    shell_service_reap();
    uint32_t wanted=ntohl(r.pid);
    for(int i=0;i<MAX_JOBS;i++) if(jobs[i].pid>0 && (uint32_t)jobs[i].pid==wanted) {
        if(kill(-jobs[i].pid,SIGKILL)<0) {
            control_reply(fd,0,0,errno); return;
        }
        jobs[i].stopping=1;
        control_reply(fd,1,1,0); return;
    }
    control_reply(fd,0,0,ESRCH);
}
void shell_service_killall(int fd)
{
    shell_service_reap();
    unsigned count=0; int error=0;
    for(int i=0;i<MAX_JOBS;i++) if(jobs[i].pid) {
        if(kill(-jobs[i].pid,SIGKILL)==0) {
            jobs[i].stopping=1; count++;
        } else if(!error) error=errno;
    }
    control_reply(fd,error==0,count,error);
}
