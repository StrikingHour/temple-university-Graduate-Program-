#include "tuplelib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc,char **argv) {
 if(argc<3)return 2;
 if(!strcmp(argv[1],"put")) {if(argc!=4)return 2;if(tuple_put(argv[2],argv[3],strlen(argv[3])+1)) {perror("put");return 1;}return 0;}
 void *p;size_t n;if(tuple_fetch(argv[2],!strcmp(argv[1],"get"),&p,&n)) {perror("fetch");return 1;}
 fwrite(p,1,n,stdout);free(p);return 0;
}
