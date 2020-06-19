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

    int port, sock;
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct  sockaddr *)&server;
    struct hostent *rem;
    char* ServerAddress = strdup(argv[8]);
    int ServerPort = atoi(argv[6]);
    char *inputString = NULL, *buffer = NULL;
    char *tmp;
    size_t size = 0;
    char *queryFile = strdup(argv[2]);
    FILE* fp = fopen(queryFile, "r");


    /*  Create  socket  */
    if ((sock = socket(AF_INET , SOCK_STREAM , 0)) < 0) {
        perror_exit("socket");
    }    
    
    /*  Find  server  address  */
    if ((rem = gethostbyname(ServerAddress)) == NULL) {
        herror("gethostbyname");
        exit (1);
    }

    port = ServerPort;
    server.sin_family = AF_INET; /*  Internet  domain  */
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port);/*  Server  port  */
    
    /*  Initiate  connection  */
    if (connect(sock , serverptr , sizeof(server)) < 0)
        perror_exit("connect");
    
    printf("Connecting to %d port %d\n", ServerPort, port);

    // printf("Give  input  string: ");
    // while( getline(&buffer, &size, stdin)!=-1 ){
    //     if( strcmp(buffer, "\n")!=0 ){
    //         inputString = strtok(buffer, "\n");
    //     }
    //     else{
    //         free(buffer);
    //     }
    //     if( inputString==NULL ){ tmp = strdup("$$$"); }
    //     else{ tmp = strdup(inputString); }
    //     if(tmp==NULL){
    //         printf("String failure.\n");
    //         return -1;
    //     }
    //     if(strcmp(tmp, "/exit")==0) {
    //         printf("exiting\n");
    //         break;
    //     }
    //     else {
    //         // if (sendMessageSock(sock , tmp) < 0) {
    //         //     perror_exit("write");/*  receive i-th  character  transformed  */
    //         // }
    //     }
    //     free(tmp);
    //     tmp = NULL;
    //     free(inputString);
    //     inputString = NULL;
    //     buffer = NULL;
    //     printf("Give  input  string: ");
    // }
    // free(tmp);
    // free(inputString);
    // buffer = NULL;

    // read querries
    while( getline(&buffer, &size, fp) >=0 ) {

        printf("Query: %s\n", buffer);

        if (sendMessageSock(sock , buffer) < 0) {
            perror_exit("write");/*  receive i-th  character  transformed  */
        }
    }

    free(buffer);
    buffer = NULL;
    
    sendMessageSock(sock , "OK");
    
    close(sock);/*  Close  socket  and  exit  */
    free(ServerAddress);
    free(queryFile);
    fclose(fp);

    return 0;
}