#define _POSIX_C_SOURCE 200809L
#include "tshlib.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
static int io_all(int fd, void *buffer, size_t size, int send_data)
{
    char *p=buffer;
    while(size) {
        ssize_t n=send_data?send(fd,p,size,MSG_NOSIGNAL):recv(fd,p,size,0);
        if(n<0 && errno==EINTR) continue;
        if(n<=0) {if(n==0) errno=ECONNRESET;return -1;}
        p+=n;size-=(size_t)n;
    }
    return 0;
}
static int finish(int fd,int result)
{
    int saved=errno;close(fd);errno=saved;return result;
}
static int begin(const char *host,uint16_t port,uint16_t op)
{
    if(!host || !*host || !port) {errno=EINVAL;return -1;}
    char service[6];snprintf(service,sizeof service,"%u",port);
    struct addrinfo hints={0},*addresses=NULL;
    hints.ai_family=AF_UNSPEC;hints.ai_socktype=SOCK_STREAM;
    int rc=getaddrinfo(host,service,&hints,&addresses);
    if(rc) {if(rc!=EAI_SYSTEM)errno=EHOSTUNREACH;return -1;}
    int fd=-1,saved=ECONNREFUSED;
    for(struct addrinfo *a=addresses;a;a=a->ai_next) {
        fd=socket(a->ai_family,a->ai_socktype,a->ai_protocol);
        if(fd<0) {saved=errno;continue;}
        if(connect(fd,a->ai_addr,a->ai_addrlen)==0) break;
        saved=errno;close(fd);fd=-1;
    }
    freeaddrinfo(addresses);
    if(fd<0) {errno=saved;return -1;}
    uint16_t wire=htons(op);
    if(io_all(fd,&wire,sizeof wire,1)<0) return finish(fd,-1);
    return fd;
}
static int server_ok(uint32_t success,uint32_t error)
{
    if(ntohl(success)) return 0;
    errno=ntohl(error)?(int)ntohl(error):EIO;return -1;
}
int tsh_shell(const char *host,uint16_t port,const char *command,
              int background,tsh_job_result *result)
{
    if(!command || !result || !*command || strlen(command)>=TSH_COMMAND_SIZE ||
       (background!=0 && background!=1)) {errno=EINVAL;return -1;}
    memset(result,0,sizeof *result);
    tsh_shell_request req={0};req.background=htonl((uint32_t)background);
    strcpy(req.command,command);
    int fd=begin(host,port,TSH_OP_SHELL);if(fd<0)return -1;
    tsh_shell_response res;
    if(io_all(fd,&req,sizeof req,1)<0 || io_all(fd,&res,sizeof res,0)<0)
        return finish(fd,-1);
    if(server_ok(res.accepted,res.error_number)<0)return finish(fd,-1);
    result->pid=ntohl(res.pid);result->completed=ntohl(res.completed);
    result->exit_code=ntohl(res.exit_code);
    return finish(fd,0);
}
int tsh_ps(const char *host,uint16_t port,tsh_job_info *jobs,
           size_t capacity,size_t *count)
{
    if(!count || (capacity && !jobs)) {errno=EINVAL;return -1;}
    *count=0;
    int fd=begin(host,port,TSH_OP_PS);if(fd<0)return -1;
    tsh_control_response res;
    if(io_all(fd,&res,sizeof res,0)<0)return finish(fd,-1);
    if(server_ok(res.success,res.error_number)<0)return finish(fd,-1);
    size_t total=ntohl(res.count);
    if(total>4096) {errno=EPROTO;return finish(fd,-1);}
    *count=total;
    for(size_t i=0;i<total;i++) {
        tsh_ps_entry row;
        if(io_all(fd,&row,sizeof row,0)<0)return finish(fd,-1);
        if(i<capacity) {
            jobs[i].pid=ntohl(row.pid);jobs[i].background=ntohl(row.background);
            jobs[i].stopping=ntohl(row.stopping);
            memcpy(jobs[i].command,row.command,TSH_COMMAND_SIZE);
            jobs[i].command[TSH_COMMAND_SIZE-1]=0;
        }
    }
    if(total>capacity) {errno=ENOSPC;return finish(fd,-1);}
    return finish(fd,0);
}
int tsh_kill(const char *host,uint16_t port,uint32_t pid)
{
    if(!pid || pid>2147483647U) {errno=EINVAL;return -1;}
    int fd=begin(host,port,TSH_OP_KILL);if(fd<0)return -1;
    tsh_kill_request req={htonl(pid)};tsh_control_response res;
    if(io_all(fd,&req,sizeof req,1)<0 || io_all(fd,&res,sizeof res,0)<0)
        return finish(fd,-1);
    return finish(fd,server_ok(res.success,res.error_number));
}
int tsh_killall(const char *host,uint16_t port,uint32_t *count)
{
    if(!count) {errno=EINVAL;return -1;}*count=0;
    int fd=begin(host,port,TSH_OP_KILLALL);if(fd<0)return -1;
    tsh_control_response res;
    if(io_all(fd,&res,sizeof res,0)<0)return finish(fd,-1);
    *count=ntohl(res.count);
    return finish(fd,server_ok(res.success,res.error_number));
}
