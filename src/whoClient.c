#include "../headers/pipes.h"
#include "../headers/general.h"
#include "../headers/ServerClient.h"

int sock;
char *buffer;

pthread_mutex_t mutexForThread = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condVar = PTHREAD_COND_INITIALIZER;

pthread_t *Threadpool = NULL;
threadQueuePtr myThreadQue = NULL;

void *handleThreads(void *Pnewsock);
// void handleQuerries(char* querry);

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
    int port;
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct  sockaddr *)&server;
    struct hostent *rem;
    char* ServerAddress = strdup(argv[8]);
    int ServerPort = atoi(argv[6]);
    int numThreads = atoi(argv[4]);
    buffer = NULL;
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
    
    for(int i=0; i<numThreads; i++) {
        pthread_create(&Threadpool[i], NULL, handleThreads, NULL);
    }

    printf("Connecting to %d\n", ServerPort);

    // read querries
    while( getline(&buffer, &size, fp) >=0 ) {

        pthread_mutex_lock(&mutexForThread);

        printf("Query: %s\n", buffer);
        if (sendMessageSock(sock , buffer) < 0) {
            perror_exit("write");/*  receive i-th  character  transformed  */
        }
        
        pthread_cond_signal(&condVar);
        pthread_mutex_unlock(&mutexForThread);
    }

    free(buffer);
    buffer = NULL;
    
    pthread_cond_broadcast(&condVar);

    for(int i=0; i<numThreads; i++) {
        pthread_join(Threadpool[i], NULL);
    }
    free(Threadpool);

    sendMessageSock(sock , "OK");
    
    close(sock);/*  Close  socket  and  exit  */
    free(ServerAddress);
    free(queryFile);
    fclose(fp);


    return 0;
}

void *handleThreads(void *Pnewsock) {
    while(true) {
        pthread_mutex_lock(&mutexForThread);

        printf("I AM IN FIRST\n");

        pthread_cond_wait(&condVar, &mutexForThread);

        printf("I AM IN\n");

        // printf("Query: %s\n", buffer);

        // if (sendMessageSock(sock , buffer) < 0) {
        //     perror_exit("write");/*  receive i-th  character  transformed  */
        // }

        pthread_mutex_unlock(&mutexForThread);
    }
    return NULL;
}