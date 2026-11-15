#define _DEFAULT_SOURCE
#include "synergy.h"
#include "tuplelib.h"
#include <stdint.h>
#define MAX_BYTES (512u * 1024u * 1024u)
static int transfer(int fd, void *buf, size_t n, int writing) {
    char *p=buf;
    while(n) {
        ssize_t k=writing?send(fd,p,n,MSG_NOSIGNAL):recv(fd,p,n,0);
        if(k<0 && errno==EINTR) continue;
        if(k<=0) {if(!k) errno=ECONNRESET; return -1;}
        p+=k; n-=(size_t)k;
    }
    return 0;
}
static int begin(unsigned short op) {
    const char *s=getenv("TSHPORT"), *host=getenv("TSHHOST"); char *end;
    if(!s || !*s || (host && strcmp(host,"127.0.0.1") && strcmp(host,"localhost"))) {errno=EINVAL;return -1;}
    long p=strtol(s,&end,10);
    if(*end || p<1 || p>65535) {errno=EINVAL;return -1;}
    int fd=socket(AF_INET,SOCK_STREAM,0); if(fd<0)return -1;
    struct sockaddr_in a={0};a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);a.sin_port=htons((unsigned short)p);
    unsigned short wire=htons(op);
    if(connect(fd,(void*)&a,sizeof(a))<0 || transfer(fd,&wire,sizeof(wire),1)<0) {int e=errno;close(fd);errno=e;return -1;}
    return fd;
}
int tuple_put(const char *name,const void *data,size_t bytes) {
    if(!name || strlen(name)>=TUPLENAME_LEN || !data || !bytes || bytes>MAX_BYTES) {errno=EINVAL;return -1;}
    tsh_put_it q={0};strcpy(q.name,name);q.priority=htons(1);q.host=htonl(INADDR_LOOPBACK);q.proc_id=(int)htonl((uint32_t)getpid());q.length=htonl((uint32_t)bytes);
    int fd=begin(TSH_OP_PUT);if(fd<0)return -1;
    tsh_put_ot r;int ok=transfer(fd,&q,sizeof(q),1);
    if(!ok)ok=transfer(fd,(void*)data,bytes,1);
    if(!ok)ok=transfer(fd,&r,sizeof(r),0);
    if(!ok && (ntohs(r.status)!=SUCCESS || (ntohs(r.error)!=TSH_ER_NOERROR && ntohs(r.error)!=TSH_ER_OVERRT))) {errno=EIO;ok=-1;}
    int e=errno;close(fd);errno=e;return ok;
}
int tuple_fetch(const char *pattern,int take,void **data,size_t *bytes) {
    if(!pattern || strlen(pattern)>=TUPLENAME_LEN || !data || !bytes) {errno=EINVAL;return -1;}
    *data=NULL;*bytes=0;
    int listener=socket(AF_INET,SOCK_STREAM,0),fd=-1,rc=-1;
    if(listener<0)return -1;
    struct sockaddr_in a={0};socklen_t alen=sizeof(a);a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    if(bind(listener,(void*)&a,sizeof(a))<0 || listen(listener,8)<0 || getsockname(listener,(void*)&a,&alen)<0)goto done;
    tsh_get_it q={0};strcpy(q.expr,pattern);q.host=a.sin_addr.s_addr;q.port=a.sin_port;q.proc_id=(int)htonl((uint32_t)getpid());
    fd=begin(take?TSH_OP_GET:TSH_OP_READ);if(fd<0)goto done;
    tsh_get_ot1 status;
    if(transfer(fd,&q,sizeof(q),1)<0 || transfer(fd,&status,sizeof(status),0)<0)goto done;
    if(ntohs(status.status)!=SUCCESS) {
        if(ntohs(status.error)!=TSH_ER_NOTUPLE) {errno=EIO;goto done;}
        close(fd);fd=-1;
        do {fd=accept(listener,NULL,NULL);}while(fd<0 && errno==EINTR);
        if(fd<0)goto done;
    }
    tsh_get_ot2 meta;
    if(transfer(fd,&meta,sizeof(meta),0)<0)goto done;
    size_t n=ntohl(meta.length);if(!n || n>MAX_BYTES) {errno=EMSGSIZE;goto done;}
    void *p=malloc(n);if(!p)goto done;
    if(transfer(fd,p,n,0)<0) {free(p);goto done;}
    *data=p;*bytes=n;rc=0;
 done: {int e=errno;if(fd>=0)close(fd);close(listener);errno=e;return rc;}
}
