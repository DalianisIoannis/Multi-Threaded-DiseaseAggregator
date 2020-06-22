#pragma once

#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ctype.h>
#include <pthread.h>

#include "./threadQueue.h"
#include "./general.h"
#include "./statistics.h"
#include "./countryList.h"

// #define SERVER_PORT 18000
#define SERVER_PORT 8989
#define SERVER_PORT2 80

#define BUFSIZE 4096
#define MAXLINE 4096

#define SOCKETERROR (-1)
#define SERVER_BACKLOG 10 // num of connections accepted


typedef struct workerStruct {
    bool hasBeenSet;
    int pidOfWorker;
    int portNum;
    char* Ipaddr;
    bool isFull;
    CountryList countriesOfWorker;

    int sock;
    struct sockaddr_in server;
    struct sockaddr *serverptr;// = (struct  sockaddr *)&server;
    struct hostent *rem;

} workerStruct;
typedef workerStruct* workerStructPtr;


typedef struct workersIdForServer {
    bool hasAcceptedFirst;
    bool hasBeenMade;
    int numOfworkers;
    int* WorkerPort;    // port for every worker to send messages
    workerStructPtr* myWorkers;
} workersIdForServer;
typedef workersIdForServer* WorkersInfo;


// #define SA struct sockaddr
typedef struct sockaddr SA;
typedef struct sockaddr_in SA_IN;

void handleConnection(int client_socket);
void inputTofirstEmpty(WorkersInfo* ar, char* pidPort, char* address, int port);
void printWorkerInfo(WorkersInfo ar);
void FinishallWorkers(WorkersInfo* ar);
void sendMsgToAllWorkers(WorkersInfo* ar, char* msg);
void connectToallWorkers(WorkersInfo* ar);
void sendToworkersFornumPatientAdmissions(WorkersInfo* ar, char* msg);
int ReturnIforCountry(WorkersInfo ar, char* country);
void sendToworkersFordiseaseFrequency(WorkersInfo* ar, char* msg);
void sendToworkersForsearchPatientRecord(WorkersInfo* ar, char* msg);
void sendToworkersFornumPatientDischarges(WorkersInfo* ar, char* msg);
void sendToworkersFortopk_AgeRanges(WorkersInfo* ar, char* msg);

int check(int exp, const char *msg);
void err_n_die(const char* fmt, ...);

void child_serverNoThread(int newsock);
void *child_serverThread(void *newsock);

void perror_exit(char *message);
void sigchld_handler (int  sig);


int sendMessageSock(int fd, char* buf);
char* receiveMessageSock(int fd, char* buf);