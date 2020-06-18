#include "../headers/pipes.h"
#include "../headers/general.h"
#include "../headers/ServerClient.h"

int main(int argc, char **argv) {

    // argv[0] ./whoClient
    // argv[1] -q
    // argv[2] queryFile
    // argv[3] –w
    // argv[4] numThreads
    // argv[5] –sp
    // argv[6] servPort
    // argv[7] –sip 
    // argv[8] servIP

    if(argv[8]==NULL){
        fprintf(stderr, "Command must be in form: ./whoServer –q queryPortNum -s statisticsPortNum –w numThreads –b bufferSize!\n");
        exit(1);
    }

    // struct sockaddr_in servaddr;
    // int sockfd, n;
    // int sendbytes;
    // char sendline[MAXLINE];
    // char recvline[MAXLINE];

    int port, sock, i;
    char buf[256];
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct  sockaddr *)&server;
    struct hostent *rem;

    char* ServerAddress = strdup(argv[8]);
    int ServerPort = atoi(argv[6]);


    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // ntoulas capitalize
    // if (argc != 3) {
    //     printf("Please  give  host  name  and  port  number\n");
    //     exit (1);
    // }


    /*  Create  socket  */
    if ((sock = socket(AF_INET , SOCK_STREAM , 0)) < 0) {
        perror_exit("socket");
    }    
    
    /*  Find  server  address  */
    if ((rem = gethostbyname(ServerAddress)) == NULL) {
        herror("gethostbyname");
        exit (1);
    }

    // port = atoi(argv [2]); /* Convert  port  number  to  integer */
    port = ServerPort;
    server.sin_family = AF_INET; /*  Internet  domain  */
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port);/*  Server  port  */
    
    /*  Initiate  connection  */
    if (connect(sock , serverptr , sizeof(server)) < 0)
        perror_exit("connect");
    
    printf("Connecting to %d port %d\n", ServerPort, port);

    do {
        printf("Give  input  string: ");
        fgets(buf , sizeof(buf), stdin);
        /*  Read  from  stdin */
        for(i=0;  buf[i] != '\0'; i++) {
            /*  For  every  char  *//*  Send i-th  character  */
            if (write(sock , buf + i, 1) < 0)
                perror_exit("write");/*  receive i-th  character  transformed  */
            
            // if (read(sock , buf + i, 1) < 0)
                // perror_exit("read");
        }
        // printf("Received  string: %s", buf);
    }
    while (strcmp(buf , "END\n") != 0);/*  Finish  on "end" */
    
    close(sock);/*  Close  socket  and  exit  */

    return 0;
}