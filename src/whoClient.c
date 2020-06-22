#include "../headers/pipes.h"
#include "../headers/general.h"
#include "../headers/signals.h"
#include "../headers/ServerClient.h"
#include "../headers/threadQueue.h"
#include <pthread.h>

typedef struct Qnode {
    struct Qnode* next;
    char* query;
} Qnode;
typedef Qnode* QnodePtr;
QnodePtr head = NULL;
QnodePtr tail = NULL;

void Enqueue(char* quer) {
    QnodePtr newNode = malloc(sizeof(Qnode));
    newNode->query = strdup(quer);
    printf("Just gave %s\n", newNode->query);
    newNode->next=NULL;
    if(tail==NULL) {
        head=newNode;
    }
    else {
        tail->next=newNode;
    }
    tail=newNode;
}

char* Dequeue() {
    printf("I am in deque\n");
    if(head==NULL) {
        return NULL;
    }
    else {
        char* res = head->query;
        QnodePtr tmp = head;
        head=head->next;
        if(head==NULL) {
            tail = NULL;
        }
        free(tmp);
        return res;
    }
}

int sock;
char *buffer;
pthread_mutex_t mutexForThread = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condVar = PTHREAD_COND_INITIALIZER;
pthread_t *ThreadpoolClient;
void *handleThreads(void *Pnewsock);

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

    printf("numThreads is %d\n", numThreads);
    if( (ThreadpoolClient = malloc(numThreads*sizeof(pthread_t)))==NULL ) {
        perror_exit("ThreadpoolClient");
    }
    
    for(int i=0; i<numThreads; i++) {
        pthread_create(&ThreadpoolClient[i], NULL, handleThreads, NULL);
    }

    printf("Connecting to %d\n", ServerPort);

    // read querries
    while( getline(&buffer, &size, fp)>=0 ) {

        pthread_mutex_lock(&mutexForThread);

        printf("Query: %s\n", buffer);
        if (sendMessageSock(sock , buffer) < 0) {
            perror_exit("write");/*  receive i-th  character  transformed  */
        }
        
        Enqueue(buffer);
        pthread_mutex_unlock(&mutexForThread);
    }

    free(buffer);
    buffer = NULL;

    for(int i=0; i<numThreads; i++) {
        pthread_join(ThreadpoolClient[i], NULL);
    }
    free(ThreadpoolClient);

    sendMessageSock(sock , "OK");
    
    close(sock);/*  Close  socket  and  exit  */
    free(ServerAddress);
    free(queryFile);
    fclose(fp);
    while(head!=NULL) {
        QnodePtr eraser = head;
        head = head->next;
        free(eraser->query);
        free(eraser);
    }
    return 0;
}

void *handleThreads(void *Pnewsock) {
    return NULL;
}