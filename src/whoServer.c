#include "../headers/pipes.h"
#include "../headers/general.h"
#include "../headers/signals.h"
#include "../headers/ServerClient.h"
#include "../headers/threadQueue.h"
#include <pthread.h>

pthread_mutex_t mutexForThreadFunc = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexForArrOfSock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexForMakingWorkAr = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

pthread_t *Threadpool = NULL;
threadQueuePtr myThreadQue = NULL;

WorkersInfo myWorkArray;

void *handleConnections(void *Pnewsock);
int setupServer(short port, int backlog);
void handleQuerries(char* querry);

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

    int ArrayOfSocketFunc[1024];
    for(int i=0; i<1024; i++) {
        ArrayOfSocketFunc[i] = 0;
    }
    myWorkArray = NULL;
    int newWorker, newClient;
    struct sockaddr_in client, worker;
    socklen_t workerlen = 0;
    socklen_t clientlen = 0;
    struct sockaddr *clientPtr = (struct sockaddr*) &client;
    struct sockaddr *workerPtr = (struct sockaddr*) &worker;
    int queryPort = atoi(argv[2]);
    int statsPort = atoi(argv[4]);
    int numThreads = atoi(argv[6]);
    int bufferSize = atoi(argv[8]);
    totalInserts = 0;

    struct sigaction *act = malloc(sizeof(struct sigaction));
    fd_set readyDescriptors, currentDescriptors;

    myWorkArray = malloc(sizeof(workersIdForServer));
    myWorkArray->hasBeenMade = false;
    myWorkArray->hasAcceptedFirst = false;

    if(queryPort==statsPort) {
        fprintf(stderr, "queryPort and statsPort must be different!\n");
        exit(1);
    }

    myThreadQue = newQueue(bufferSize);

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

                        int *pclient = malloc(sizeof(int));
                        *pclient = i;

                        pthread_mutex_lock(&mutexForThreadFunc);

                        enqueue(&myThreadQue, pclient, ArrayOfSocketFunc[i]);
                        FD_CLR(i, &currentDescriptors);
                        
                        pthread_cond_signal(&cond_var);
                        // while((*Queue)->count >= (*Queue)->bufferSize) {
                        //     // found full
                        //     pthread_cond_wait(&cond_nonfull, &mtx);
                        // }
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
    close(workerSocket);    // should be here?
    close(clientSocket);    // should be here?
    delQueue(&myThreadQue);
    free(act);
    for(int i=0; i<myWorkArray->numOfworkers; i++) {
        
        emptycountryList(&(((myWorkArray->myWorkers)[i])->countriesOfWorker));
        free(((myWorkArray->myWorkers)[i])->countriesOfWorker);
        free(((myWorkArray->myWorkers)[i])->Ipaddr);
        free( (myWorkArray->myWorkers)[i] );
    }
    free(myWorkArray->myWorkers);
    free(myWorkArray);

    return 0;
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
                char* readed;

                // take statistics
                for( ; ; ) {

                    char arr[100];
                    char* readed = receiveMessageSock(tmpSock, arr);
                    if(strcmp(readed, "OK")==0){
                        printf("Received stats\n");
                        free(readed);
                        break;
                    }

                    printStatsFromConcat(readed);

                    free(readed);

                }
                printf("OUT OF STATS\n");

                // // receive numWorkers
                readed = receiveMessageSock(tmpSock, buf);

                pthread_mutex_lock(&mutexForMakingWorkAr);
                if(myWorkArray->hasBeenMade==false) {   // first worker
                    
                    myWorkArray->hasBeenMade=true;
                    char* tmpNumWorks = strdup(readed);
                    myWorkArray->numOfworkers = atoi(readed);
                    myWorkArray->WorkerPort = malloc((myWorkArray->numOfworkers)*sizeof(int));
                    
                    for(int i=0; i<(myWorkArray->numOfworkers); i++) {
                        (myWorkArray->WorkerPort)[i] = 0;
                    }

                    myWorkArray->myWorkers = malloc((myWorkArray->numOfworkers)*sizeof(workerStructPtr));
                    for(int i=0; i<(myWorkArray->numOfworkers); i++) {
                        ((myWorkArray->myWorkers)[i]) = malloc(sizeof(workerStruct));
                        ((myWorkArray->myWorkers)[i])->hasBeenSet = false;
                        ((myWorkArray->myWorkers)[i])->pidOfWorker = -1;
                        ((myWorkArray->myWorkers)[i])->portNum = -1;
                        ((myWorkArray->myWorkers)[i])->countriesOfWorker = initcountryList();
                    }

                    free(tmpNumWorks);
                }
                free(readed);

                pthread_mutex_unlock(&mutexForMakingWorkAr);
                // receive port and IPaddress
                readed = receiveMessageSock(tmpSock, buf);
                char* givePort = strdup(readed);
                free(readed);

                readed = receiveMessageSock(tmpSock, buf);
                char* giveAdr = strdup(readed);

                printf("Received port %s and address %s\n", givePort, giveAdr);
                inputTofirstEmpty(&myWorkArray, givePort, giveAdr, tmpSock);

                free(giveAdr);
                free(givePort);
                free(readed);          

                sendMessageSock(tmpSock, "Server received statistics.");
                close(tmpSock);
            }
            else {  // ISCLIENT

                int tmpSock = tmp->qSocket;
                delThreadNode(&tmp);

                connectToallWorkers(&myWorkArray);

                for( ; ; ) {

                    char arr[100];
                    char* readed = receiveMessageSock(tmpSock, arr);
                    if(strcmp(readed, "OK")==0){
                        free(readed);
                        break;
                    }

                    printf("Querry: %s\n", readed);
                    
                    handleQuerries(readed);
                    
                    free(readed);

                }

                FinishallWorkers(&myWorkArray);
                printf("Closed connection to client!\n");
                close(tmpSock);
            }
        }
    }
    return NULL;
}

void handleQuerries(char* querry) {

    char* firstQ = strdup(querry);
    char* instruct = strtok(querry," ");
    char* ind5 = strtok(NULL," ");
    ind5 = strtok(NULL," ");
    ind5 = strtok(NULL," ");
    ind5 = strtok(NULL," ");
    ind5 = strtok(NULL," ");

    if( strcmp(instruct, "/diseaseFrequency")==0 ){ // diseaseFrequency virusName date1 date2 [country]

        sendToworkersFordiseaseFrequency(&myWorkArray, firstQ);

    }
    else if(strcmp(instruct, "/topk-AgeRanges")==0) {
        if( ind5==NULL ){
            printf("Need to provide proper variables.\n");
        }
        else {

            sendToworkersFortopk_AgeRanges(&myWorkArray, firstQ);

        }
    }
    else if(strcmp(instruct, "/searchPatientRecord")==0) {

        sendToworkersForsearchPatientRecord(&myWorkArray, firstQ);

    }
    else if(strcmp(instruct, "/numPatientAdmissions")==0) {
        sendToworkersFornumPatientAdmissions(&myWorkArray, firstQ);
    }
    else if(strcmp(instruct, "/numPatientDischarges")==0) {
        sendToworkersFornumPatientDischarges(&myWorkArray, firstQ);
    }

    free(firstQ);
    
    return;
}


int setupServer(short port, int backlog) {
    int sSocket;
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