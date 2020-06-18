#include "../headers/pipes.h"
#include "../headers/general.h"
#include "../headers/signals.h"
#include "../headers/ServerClient.h"
#include "../headers/threadQueue.h"
#include <pthread.h>

pthread_mutex_t mutexForThreadFunc = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexForArrOfSock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

void* threadFunc(void* arg);
void *handleConnections(void *Pnewsock);

pthread_t *Threadpool = NULL;
threadQueuePtr myThreadQue = NULL;

int Termination = 0;
int Sigkill = 0;
int MySignalFlagForSIGINT_SIGQUIT=0;

int setupServer(short port, int backlog);

void Myhandler(int sig, siginfo_t* siginfo, void* buf);
void ServerHandler(struct sigaction *act, void (*Myhandler)(int, siginfo_t*, void*));

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

    // int statSocket;
    // SA_IN server_addr;
    // sigset_t emptSet;
    // uint8_t buff[MAXLINE+1];
    // uint8_t recvline[MAXLINE+1];
    int ArrayOfSocketFunc[1024];
    for(int i=0; i<1024; i++) {
        ArrayOfSocketFunc[i] = 0;
    }
    int sock;
    int newWorker, newClient;
    struct sockaddr_in client, worker;
    socklen_t workerlen = 0;
    socklen_t clientlen = 0;
    struct sockaddr *clientPtr = (struct sockaddr*) &client;
    struct sockaddr *workerPtr = (struct sockaddr*) &worker;
    struct hostent *rem;

    struct sigaction *act = malloc(sizeof(struct sigaction));
    // ServerHandler(act, Myhandler);
    fd_set readyDescriptors, currentDescriptors;

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
        pthread_create(&Threadpool[i], NULL, handleConnections, NULL);
    }

    int workerSocket = setupServer(statsPort, SERVER_BACKLOG);
    int clientSocket = setupServer(queryPort, SERVER_BACKLOG);

    FD_ZERO(&currentDescriptors);
    FD_SET(workerSocket, &currentDescriptors);
    FD_SET(clientSocket, &currentDescriptors);
    int maxfd = returnMaxInt(workerSocket, clientSocket) + 1;

    printf("Listening from statsPort %d\n", statsPort);
    printf("Listening from queryPort %d\n", queryPort);

    while  (1) {

        readyDescriptors = currentDescriptors;

        if( pselect(maxfd, &readyDescriptors, NULL, NULL, NULL, NULL)<0 ) {
            perror("Select error");
            return -1;
        }
        else {

            for(int i=0; i<maxfd; i++) {

                if(FD_ISSET(i, &readyDescriptors)) {

                    if(i==workerSocket) {

                        /*  accept  connection  */
                        if (( newWorker = accept(workerSocket, workerPtr, &workerlen)) < 0) {
                            perror_exit("accept");
                        }
                        
                        // if(MySignalFlagForSIGINT_SIGQUIT==-1 || MySignalFlagForSIGINT_SIGQUIT==-2) {
                        //     MySignalFlagForSIGINT_SIGQUIT=0;
                        //     break;
                        // }

                        printf("Accepted worker connection at %d\n", newWorker);

                        FD_SET(newWorker, &currentDescriptors);
                        maxfd = returnMaxInt(maxfd, newWorker) + 1;
                        
                        pthread_mutex_lock(&mutexForArrOfSock);
                        ArrayOfSocketFunc[newWorker] = ISWORKER;
                        pthread_mutex_unlock(&mutexForArrOfSock);


                    }
                    else if(i==clientSocket) {

                        /*  accept  connection  */
                        if (( newClient = accept(clientSocket, clientPtr, &clientlen)) < 0) {
                            perror_exit("accept");
                        }

                        printf("Accepted client connection at %d\n", newClient);

                        FD_SET(newClient, &currentDescriptors);
                        maxfd = returnMaxInt(maxfd, newClient) + 1;

                        pthread_mutex_lock(&mutexForArrOfSock);
                        ArrayOfSocketFunc[newClient] = ISQUERY;
                        pthread_mutex_unlock(&mutexForArrOfSock);

                    }
                    else {

                        memset(&worker, 0, sizeof(struct sockaddr_in));
                        socklen_t sa_len = sizeof(struct sockaddr_in);
                        int tmpRealPort = getsockname(i, (struct sockaddr *)&worker, &sa_len);
                        printf("In pid %d returned %d\n", getpid(), tmpRealPort);
                        char* toNtoa = strdup(inet_ntoa(worker.sin_addr));
                        printf("In pid %d toNtoa %s\n", getpid(), toNtoa);
                        int ntoHs = ntohs(worker.sin_port);
                        printf("In pid %d ntoHs %d\n", getpid(), ntoHs);

                        int *pclient = malloc(sizeof(int));
                        *pclient = i;
                        pthread_mutex_lock(&mutexForThreadFunc);

                        enqueue(&myThreadQue, pclient, ArrayOfSocketFunc[i]);
                        
                        FD_CLR(i, &currentDescriptors);
                        
                        pthread_cond_signal(&cond_var);
                        
                        pthread_mutex_unlock(&mutexForThreadFunc);

                    }
                }
            }
        }

    }

    pthread_cond_broadcast(&cond_var);

    for(int i=0; i<numThreads; i++) {
        pthread_join(Threadpool[i], NULL);
    }

    free(Threadpool);

    close(workerSocket); // should be here?

    delQueue(&myThreadQue);

    free(act);

    return 0;
}


void* threadFunc(void* arg) {

    qNodePtr tmp = NULL;
    while(true) {
        pthread_mutex_lock(&mutexForThreadFunc);

        if( (tmp=dequeue(&myThreadQue))==NULL ) {
            pthread_cond_wait(&cond_var, &mutexForThreadFunc);
            // try again
            tmp = dequeue(&myThreadQue);
        }

        pthread_mutex_unlock(&mutexForThreadFunc);
        if(tmp!=NULL) {
            int tmpSock = tmp->qSocket;
            delThreadNode(&tmp);
            child_serverNoThread(tmpSock);
        }
    }
}

// check if is worker or client
void *handleConnections(void *Pnewsock) {

    qNodePtr tmp = NULL;
    while(true) {
        pthread_mutex_lock(&mutexForThreadFunc);

        if( (tmp=dequeue(&myThreadQue))==NULL ) {
            pthread_cond_wait(&cond_var, &mutexForThreadFunc);
            // try again
            tmp = dequeue(&myThreadQue);
        }

        pthread_mutex_unlock(&mutexForThreadFunc);
        if(tmp!=NULL) {
            
            if(tmp->identity==ISWORKER) {
                
                int tmpSock = tmp->qSocket;

                delThreadNode(&tmp);

                char buf[256] = {0};
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
                    // printStatsFromConcat(readed);
                    free(readed);

                }

                sendMessageSock(tmpSock, "Server received statistics.");
                close(tmpSock);
            }
            else {
                int tmpSock = tmp->qSocket;

                delThreadNode(&tmp);

                // take random input
                for( ; ; ) {

                    char arr[100];
                    char* readed = receiveMessageSock(tmpSock, arr);
                    printf("Received %s\n", readed);
                    if(strcmp(readed, "OK")==0){
                        free(readed);
                        break;
                    }
                    free(readed);

                }

                // sendMessageSock(tmpSock, "Server received statistics.");
                printf("Closed connection to client!\n");
                close(tmpSock);
            }
        }
    }
    return NULL;
}

void Myhandler(int sig, siginfo_t* siginfo, void* buf) {
    if(sig==SIGINT || sig==SIGQUIT) {
        printf("Child %d caught SIGINT or SIGQUIT.\n", getpid());
        MySignalFlagForSIGINT_SIGQUIT=-1;
    }
    else if(sig==SIGUSR1) {
        printf("Child %d caught SIGUSR1.\n", getpid());
        MySignalFlagForSIGINT_SIGQUIT=-2;
    }
}

void ServerHandler(struct sigaction *act, void (*Myhandler)(int, siginfo_t*, void*)) {

    act->sa_handler = (void*)Myhandler;
    act->sa_sigaction = Myhandler;
    sigfillset(&act->sa_mask);
    act->sa_flags = SA_NODEFER | SA_RESTART | SA_SIGINFO;
    
    sigaction(SIGUSR1, act, NULL);
    sigaction(SIGINT, act, NULL);
    sigaction(SIGQUIT, act, NULL);
}


int setupServer(short port, int backlog) {
    int sSocket, cSocket, addrSize;
    SA_IN server_addr;

    check( (sSocket = socket(AF_INET, SOCK_STREAM, 0)), "Failed Socket");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(sSocket , (SA*)&server_addr , sizeof(server_addr)) < 0) {
        perror_exit("bind");
    }
    
    /*  Listen  for  connections  */
    if (listen(sSocket , backlog) < 0) {
        perror_exit("listen");
    }

    return sSocket;
}