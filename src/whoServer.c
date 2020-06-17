#include "../headers/pipes.h"
#include "../headers/general.h"
#include "../headers/signals.h"
#include "../headers/ServerClient.h"
#include "../headers/threadQueue.h"
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

// int bufferSize;

void* threadFunc(void* arg);
void *printTry(void *Pnewsock);

pthread_t *Threadpool = NULL;

threadQueuePtr myThreadQue = NULL;

int main(int argc, char **argv) {

    // argv[0] ./whoServer
    // argv[1] -q
    // argv[2] queryPortNum
    // argv[3] –s
    // argv[4] statisticsPortNum
    // argv[5] –w
    // argv[6] numThreads
    // argv[7] –b
    // argv[8] bufferSize

    if(argv[8]==NULL){
        fprintf(stderr, "Command must be in form: ./whoServer –q queryPortNum -s statisticsPortNum –w numThreads –b bufferSize!\n");
        exit(1);
    }

    int server_socket, client_socket, addr_size;
    int querySocket, statSocket;
    int listenfd, connfd, n;
    struct sockaddr_in servaddr;
    SA_IN server_addr, client_addr;
    // uint8_t buff[MAXLINE+1];
    // uint8_t recvline[MAXLINE+1];

    int sock;
    int newsock;
    struct sockaddr_in server, client;
    socklen_t clientlen = 0;
    struct sockaddr *serverptr = (struct sockaddr*) &server;
    struct sockaddr *clientptr = (struct sockaddr*) &client;
    struct hostent *rem;

    struct sigaction act;
    fd_set fds1, fds2;

    int queryPort = atoi(argv[2]);
    int statsPort = atoi(argv[4]);
    int numThreads = atoi(argv[6]);
    int bufferSize = atoi(argv[8]);

    if(queryPort==statsPort) {
        fprintf(stderr, "queryPort and statsPort must be different!\n");
        exit(1);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // connect to workers

    myThreadQue = newQueue();

    if( (Threadpool = malloc(numThreads*sizeof(pthread_t)))==NULL ) {
        perror_exit("Threadpool");
    }

    for(int i=0; i<numThreads; i++) {
        pthread_create(&Threadpool[i], NULL, printTry, NULL);
    }

    int workerSocket;
    if ((workerSocket = socket(AF_INET , SOCK_STREAM , 0)) < 0) {
        perror_exit("socket");
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(statsPort);

    /*  Bind  socket  to  address  */
    if (bind(workerSocket , serverptr , sizeof(server)) < 0) {
        perror_exit("bind");
    }
    
    /*  Listen  for  connections  */
    if (listen(workerSocket , SERVER_BACKLOG) < 0) {
        perror_exit("listen");
    }

    printf("Listening  for  connections  to port %d\n", statsPort);
    while  (1) {

        /*  accept  connection  */
        if (( newsock = accept(workerSocket, clientptr, &clientlen)) < 0) {
            perror_exit("accept");
        }

        // memset(&server, 0, sizeof(struct sockaddr_in));
        // socklen_t sa_len = sizeof(struct sockaddr_in);
        // int tmpRealPort = getsockname(workerSocket, (struct sockaddr *)&server, &sa_len);
        // printf("In pid %d returned %d\n", getpid(), tmpRealPort);
        // char* toNtoa = strdup(inet_ntoa(server.sin_addr));
        // printf("In pid %d toNtoa %s\n", getpid(), toNtoa);
        // int ntoHs = ntohs(server.sin_port);
        // printf("In pid %d ntoHs %d\n", getpid(), ntoHs);

        printf("Accepted  connection.\n");
        
        // printTry(newsock);

        int *pclient = malloc(sizeof(int));
        *pclient = newsock;
        pthread_mutex_lock(&mutex);

        enqueue(&myThreadQue, pclient);
        pthread_cond_signal(&cond_var);
        
        pthread_mutex_unlock(&mutex);

    }

    pthread_cond_broadcast(&cond_var);

    for(int i=0; i<numThreads; i++) {
        pthread_join(Threadpool[i], NULL);
    }

    free(Threadpool);

    close(workerSocket); // should be here?

    delQueue(&myThreadQue);


    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // ntoulas capitalize
    
    // myThreadQue = newQueue();

    // if( (Threadpool = malloc(numThreads*sizeof(pthread_t)))==NULL ) {
    //     perror_exit("Threadpool");
    // }

    // for(int i=0; i<numThreads; i++) {
    //     pthread_create(&Threadpool[i], NULL, threadFunc, NULL);
    // }
    
    // /*  Reap  dead  children  asynchronously  */
    // signal(SIGCHLD , sigchld_handler);

    // /*  Create  socket  for queryPortNum */
    // if ((statSocket = socket(AF_INET , SOCK_STREAM , 0)) < 0) {
    //     perror_exit("socket");
    // }

    // server.sin_family = AF_INET;
    // server.sin_addr.s_addr = htonl(INADDR_ANY);
    // server.sin_port = htons(statsPort);

    // /*  Bind  socket  to  address  */
    // if (bind(statSocket , serverptr , sizeof(server)) < 0) {
    //     perror_exit("bind");
    // }
    
    // /*  Listen  for  connections  */
    // if (listen(statSocket , SERVER_BACKLOG) < 0) {
    //     perror_exit("listen");
    // }

    // // FD_ZERO(&fds1);
    // // FD_SET(statSocket, &fds1)
    // // maxfd = returnMaxInt()

    // printf("Listening  for  connections  to port %d\n", statsPort);
    // while  (1) {

    //     /*  accept  connection  */
    //     if (( newsock = accept(statSocket, clientptr, &clientlen)) < 0) {
    //         perror_exit("accept");
    //     }

    //     printf("Accepted  connection.\n");
    //     // switch (fork()) {   /*  Create  child  for  serving  client  */
    //     //     case  -1:       /*  Error  */
    //     //         perror("fork");
    //     //         break;
    //     // case 0:             /*  Child  process  */
    //     //     close(statSocket);
    //     //     child_serverNoThread(newsock);
    //     //     exit (0);
    //     // }
    //     // close(newsock);     /*  parent  closes  socket  to  client */


        
    //     // child_serverNoThread(newsock);

    //     // pthread_t t;
    //     // int *pclient = malloc(sizeof(int));
    //     // *pclient = newsock;
    //     // pthread_create(&t, NULL, child_serverThread, pclient);

    //     int *pclient = malloc(sizeof(int));
    //     *pclient = newsock;
    //     pthread_mutex_lock(&mutex);

    //     enqueue(&myThreadQue, pclient);
    //     pthread_cond_signal(&cond_var);
        
    //     pthread_mutex_unlock(&mutex);

    // }

    // pthread_cond_broadcast(&cond_var);

    // for(int i=0; i<numThreads; i++) {
    //     pthread_join(Threadpool[i], NULL);
    // }

    // close(statSocket); // should be here?
    
    // free(Threadpool);

    // delQueue(&myThreadQue);

    f();

    return 0;
}


void* threadFunc(void* arg) {

    qNodePtr tmp = NULL;

    while(true) {
        pthread_mutex_lock(&mutex);

        if( (tmp=dequeue(&myThreadQue))==NULL ) {
            pthread_cond_wait(&cond_var, &mutex);
            // try again
            tmp = dequeue(&myThreadQue);
        }
        
        pthread_mutex_unlock(&mutex);
        if(tmp!=NULL) {
            
            int tmpSock = tmp->qSocket;

            delThreadNode(&tmp);

            child_serverNoThread(tmpSock);
        }
    }
}

void *printTry(void *Pnewsock) {

    qNodePtr tmp = NULL;
    while(true) {
        pthread_mutex_lock(&mutex);

        if( (tmp=dequeue(&myThreadQue))==NULL ) {
            pthread_cond_wait(&cond_var, &mutex);
            // try again
            tmp = dequeue(&myThreadQue);
        }

        pthread_mutex_unlock(&mutex);
        if(tmp!=NULL) {
            
            int tmpSock = tmp->qSocket;

            delThreadNode(&tmp);

            char buf[256] = {0};
            // read(tmpSock, buf, strlen("geia"));
            char* readed = receiveMessageSock(tmpSock, buf);
            printf("Gave %s\n", readed);
            free(readed);
            printf("Closing  connection .\n");

            // take statistics
            for( ; ; ) {

                char arr[100];
                char* readed = receiveMessageSock(tmpSock, arr);

                if(strcmp(readed, "OK")==0){
                    free(readed);
                    break;
                }

                printStatsFromConcat(readed);
                free(readed);

            }
            close(tmpSock);
        }
    }
    return NULL;
}