/*.........................................................................*/
/*                  TSHTEST.C ------> TSH test program                     */
/*                                                                         */
/*                  By N. Isaac Rajkumar [April '93]                       */
/*                  February '13, updated by Justin Y. Shi                 */
/*.........................................................................*/

#include "tshtest.h"
#include "tsh_shell.h"
static void TestShell(void);
static void TestProcess(int choice);
int status;

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        return 1;
    }
    for (;;) {
        int choice = drawMenu();
        if (choice == 6) return 0;
        unsigned short opcode;
        switch (choice) {
            case 1: opcode=TSH_OP_PUT; break;
            case 2: opcode=TSH_OP_GET; break;
            case 3: opcode=TSH_OP_READ; break;
            case 4: opcode=TSH_OP_EXIT; break;
            case 5: opcode=TSH_OP_SHELL; break;
            case 7: opcode=TSH_OP_PS; break;
            case 8: opcode=TSH_OP_KILL; break;
            case 9: opcode=TSH_OP_KILLALL; break;
            default: continue;
        }
        tshsock=connectTsh(atoi(argv[1]));
        unsigned short wire=htons(opcode);
        if (writen(tshsock,(char *)&wire,sizeof wire) != 1) return 1;
        switch(choice) {
            case 1: OpPut(); break;
            case 2: case 3: OpGet(choice == 3); break;
            case 4: OpExit(); break;
            case 5: TestShell(); break;
            case 7: case 8: case 9: TestProcess(choice); break;
        }
        close(tshsock);
        if (choice == 4) return 0;
    }
}

static void TestShell(void)
{
    tsh_shell_request request = {0};
    tsh_shell_response response;
    int c;
    while ((c=getchar()) != '\n' && c != EOF) {}
    printf("Command (append & for background): ");
    fflush(stdout);
    if (!fgets(request.command,sizeof request.command,stdin)) return;
    request.command[strcspn(request.command,"\n")] = 0;
    if (writen(tshsock,(char *)&request,sizeof request) != 1 ||
        readn(tshsock,(char *)&response,sizeof response) != 1) {
        fprintf(stderr,"Shell request: incomplete response\n");
    } else if (!ntohl(response.accepted)) {
        printf("Launch rejected: %s\n",strerror((int)ntohl(response.error_number)));
    } else {
        printf("Job PID: %u\n",ntohl(response.pid));
        if (ntohl(response.completed))
            printf("Completed. Exit status: %u\n",ntohl(response.exit_code));
        else
            printf("Background job accepted; completion is reported in daemon log.\n");
    }
    printf("Press Enter to return to menu."); fflush(stdout);
    while ((c=getchar()) != '\n' && c != EOF) {}
}

static void TestProcess(int choice)
{
    int c;
    while ((c=getchar()) != '\n' && c != EOF) {}
    if(choice==8) {
        char line[64], *end;
        printf("Job PID to stop (from Shell or PS): "); fflush(stdout);
        if(!fgets(line,sizeof line,stdin)) return;
        errno=0;
        unsigned long value=strtoul(line,&end,10);
        while(*end==' ' || *end=='\t' || *end=='\n') end++;
        if(errno || end==line || *end || value==0 || value>2147483647UL) {
            puts("Invalid PID."); return;
        }
        tsh_kill_request request={htonl((uint32_t)value)};
        if(!writen(tshsock,(char *)&request,sizeof request)) return;
    }
    tsh_control_response response;
    if(!readn(tshsock,(char *)&response,sizeof response)) {
        puts("Incomplete management response."); return;
    }
    unsigned count=ntohl(response.count);
    if(!ntohl(response.success)) {
        printf("Request failed: %s (affected jobs: %u)\n",
            strerror((int)ntohl(response.error_number)),count);
    } else if(choice==7) {
        printf("Active jobs: %u\n",count);
        printf("PID       MODE  STATE       COMMAND\n");
        for(unsigned i=0;i<count;i++) {
            tsh_ps_entry row;
            if(!readn(tshsock,(char *)&row,sizeof row)) return;
            row.command[TSH_COMMAND_SIZE-1]=0;
            printf("%-9u %-5s %-11s %s\n",ntohl(row.pid),
                ntohl(row.background)?"BG":"FG",
                ntohl(row.stopping)?"STOPPING":"RUNNING",row.command);
        }
    } else {
        printf("SIGKILL sent to %u tracked job group(s).\n",count);
    }
    printf("Press Enter to return to menu."); fflush(stdout);
    while((c=getchar()) != '\n' && c != EOF) {}
}


void OpPut()
{
   tsh_put_it out = {0};
   tsh_put_ot in ;
   int tmp ;
   char *buff ;

   status=system("clear") ;
   printf("TSH_OP_PUT") ;
   printf("\n----------\n") ;
				/* obtain tuple name, priority, length, */
   printf("\nEnter tuple name : ") ; /* and the tuple */
   if (scanf("%127s", out.name) != 1) {
      puts("Invalid tuple name.");
      return;
   }
   printf("Enter priority : ") ;
   if (scanf("%d", &tmp) != 1 || tmp < 0 || tmp > 65535) {
      puts("Priority must be between 0 and 65535.");
      return;
   }
   out.priority = (u_short)tmp ;
   printf("Enter length : ") ;
   if (scanf("%d", &out.length) != 1 || out.length < 2 || out.length > 1048576) {
      puts("Text tuple length must be between 2 and 1048576 bytes.");
      return;
   }
   /* Discard rest of length line, then safely read a bounded text tuple. */
   while ((tmp = getchar()) != '\n' && tmp != EOF) {}
   printf("Enter tuple : ") ;
   buff = (char *)calloc((size_t)out.length, 1) ;
   if (!buff) { perror("calloc"); return; }
   if (!fgets(buff, out.length, stdin)) {
      free(buff);
      puts("Could not read tuple text.");
      return;
   }
   /* If text exceeded the declared length, consume the remainder. */
   if (!strchr(buff, '\n')) {
      while ((tmp = getchar()) != '\n' && tmp != EOF) {}
   }
				/* print data sent to TSH */
   printf("\n\nTo TSH :\n") ;
   printf("\nname : %s", out.name) ;
   printf("\npriority : %d", out.priority) ;
   printf("\nlength : %d", out.length) ;
   printf("\ntuple : %s\n", buff) ;

   out.priority = htons(out.priority) ;
   out.length = htonl(out.length) ;
				/* send data to TSH */
   if (!writen(tshsock, (char *)&out, sizeof(out)))
    {
       perror("\nOpPut::writen\n") ;
       getchar() ;
       free(buff) ;
       return ;
    }
				/* send tuple to TSH */
   if (!writen(tshsock, buff, ntohl(out.length)))
    {
       perror("\nOpPut::writen\n") ;
       getchar() ;
       free(buff) ;
       return ;
    }
				/* read result */
   if (!readn(tshsock, (char *)&in, sizeof(in)))
    {
       perror("\nOpPut::readn\n") ;
       free(buff) ;
       getchar() ;
       return ;
    }
				/* print result from TSH */
   printf("\n\nFrom TSH :\n") ;
   printf("\nstatus : %d", ntohs(in.status)) ;
   printf("\nerror : %d\n", ntohs(in.error)) ;
   free(buff) ;
   getchar() ;
}


void OpGet(int is_read)
{
   tsh_get_it out = {0};
   tsh_get_ot1 in1 ;
   tsh_get_ot2 in2 ;
   struct in_addr addr ;
   int sd, sock ;
   char *buff ;

   status=system("clear") ;
   printf("TSH_OP_%s", is_read ? "READ" : "GET") ;
   printf("\n----------\n") ;
				/* obtain tuple name/wild card */
   printf("\nEnter tuple name [wild cards ?, * allowed] : ") ;
   if (scanf("%127s", out.expr) != 1) {
      puts("Invalid tuple pattern.");
      return;
   }
				/* obtain port for return data if tuple not available */
   // This line has to revise for clusters. out.host = gethostid() ;	
   out.host = inet_addr("127.0.0.1");
   if ((sd = get_socket()) == -1)
    {
       perror("\nOpGet::get_socket\n") ;
       getchar() ; getchar() ;
       return ;
    }
   if (!(out.port = bind_socket(sd, 0)))
    {
       perror("\nOpGet::bind_socket\n") ;
       getchar() ; getchar() ;
       return ;
    }
   addr.s_addr = out.host ;
				/* print data  sent to TSH */
   printf("\n\nTo TSH :\n") ;
   printf("\nexpr : %s", out.expr) ;
   printf("\nhost : %s", inet_ntoa(addr)) ;
   printf("\nport : %d\n", (out.port)) ;
				/* send data to TSH */
   if (!writen(tshsock, (char *)&out, sizeof(out)))
    {
       perror("\nOpGet::writen\n") ;
       getchar() ; getchar() ;
       close(sd) ;
       return ;
    }
				/* find out if tuple available */
   if (!readn(tshsock, (char *)&in1, sizeof(in1)))
    {
       perror("\nOpGet::readn\n") ;
       getchar() ; getchar() ;
       close(sd) ;
       return ;
    }
				/* print whether tuple available in TSH */
   printf("\n\nFrom TSH :\n") ;
   printf("\nstatus : %d", ntohs(in1.status)) ;
   printf("\nerror : %d\n", ntohs(in1.error)) ;
				/* if tuple is available read from the same */
   if (ntohs(in1.status) == SUCCESS) /* socket */
      sock = tshsock ;
   else				/* get connection in the return port */
      sock = get_connection(sd, NULL) ;
   if (sock < 0) {
      perror("OpGet::get_connection");
      close(sd);
      return;
   }
				/* read tuple details from TSH */
   if (!readn(sock, (char *)&in2, sizeof(in2)))
    {
       perror("\nOpGet::readn\n") ;
       getchar() ; getchar() ;
       close(sd) ;
       return ;
    }				/* print tuple details from TSH */
   printf("\nname : %s", in2.name) ;
   printf("\npriority : %d", ntohs(in2.priority)) ;
   printf("\nlength : %d", ntohl(in2.length)) ;
   int tuple_length = ntohl(in2.length);
   if (tuple_length < 0 || tuple_length > 1048576) {
      puts("Invalid or oversized text tuple reply.");
      close(sd);
      if (sock != tshsock) close(sock);
      return;
   }
   buff = (char *)malloc((size_t)tuple_length + 1);
   if (!buff) {
      perror("malloc");
      close(sd);
      if (sock != tshsock) close(sock);
      return;
   }
				/* read, print tuple from TSH, with a terminator */
   if (!readn(sock, buff, tuple_length))
      perror("\nOpGet::readn\n") ;
   else {
      buff[tuple_length] = '\0';
      printf("\ntuple : %s\n", buff);
   }

   close(sd) ;
   if (sock != tshsock) close(sock) ;
   free(buff) ;
   getchar() ; getchar() ;
}


void OpExit()
{
   tsh_exit_ot in ;
   
   status=system("clear") ;
   printf("TSH_OP_EXIT") ;
   printf("\n-----------\n") ;
				/* read TSH response */
   if (!readn(tshsock, (char *)&in, sizeof(in)))
    {
       perror("\nOpExit::readn\n") ;
       getchar() ;  getchar() ;
       return ;
    }
				/* print TSH response */
   printf("\n\nFrom TSH :\n") ;
   printf("\nstatus : %d", ntohs(in.status)) ;
   printf("\nerror : %d\n", ntohs(in.error)) ;
   getchar() ;  getchar() ;
}


int connectTsh(u_short port)
{
   short tsh_port ;
   u_long tsh_host ;
   int sock ;

   // use local host 
   tsh_host = inet_addr("127.0.0.1");
   /*
   if ((host = gethostbyname("localhost")) == NULL)
	{
	   perror("connectTsh::gethostbyname\n") ;
	   exit(1) ;
	}
   tsh_host = *((long *)host->h_addr_list[0]) ;
   */
   tsh_port = htons(port);
				/* get socket and connect to TSH */
   if ((sock = get_socket()) == -1)
    {
       perror("connectTsh::get_socket\n") ;
       exit(1) ;
    }
   if (!do_connect(sock, tsh_host, tsh_port))
    {
       perror("connectTsh::do_connect\n") ;
       exit(1) ;
    }      
   return sock ;
}


u_short drawMenu()
{
   int choice ;
				/* draw menu of user options */
   status=system("clear") ;
   printf("\n\n\n\t\t\t---------") ;
   printf("\n\t\t\tMAIN MENU") ;
   printf("\n\t\t\t---------") ;
   printf("\n\n\t\t\t 1. Put") ;
   printf("\n\t\t\t 2. Get") ;
   printf("\n\t\t\t 3. Read") ;
   printf("\n\t\t\t 4. Exit (TSH)") ;
   printf("\n\t\t\t 5. Shell") ;
   printf("\n\t\t\t 6. Quit from this program") ;
   printf("\n\t\t\t 7. PS - list daemon jobs") ;
   printf("\n\t\t\t 8. KILL - stop one job") ;
   printf("\n\t\t\t 9. KILLALL - stop all daemon jobs") ;
       
   printf("\n\n\n\t\t\tEnter Choice : ") ;

   if (scanf("%d", &choice) != 1) {
      puts("Invalid input; exiting tester.");
      return 6;
   }
   return choice ;
}
