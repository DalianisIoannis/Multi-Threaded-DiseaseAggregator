#pragma once

#include "./countryList.h"
#include "./general.h"
#include "./linkedList.h"
#include "./statistics.h"
#include "./pipes.h"
#include "./ServerClient.h"
#include "./HashTable.h"

typedef struct workerData{
    int totalCountries;

    pid_t pid;
    CountryList PIDcountries;
    char *fifoRead;
    char *fifoWrite;
    
    int fdRead;
    int fdWrite;
    char* IPaddres;
    char* PortNumber;
    
    struct workerData *next;
} workerData;
typedef workerData *workerDataNode;


typedef struct {
    int TOTAL;
    int SUCCESS;
} requestStruct;


void printWorkerNode(workerDataNode);
workerDataNode makeWorkerArCell(pid_t, int);
void emptyworkerNode(workerDataNode*);
int WorkerRun(char*, int, int, int, int);
int inputPatientsToStructures(char*, Linked_List*, char* date, char* country, StatisticsList*, Linked_List *);
void handleWorkerQuerries(int sock, HashTable HT_disease, HashTable HT_country, Linked_List ListOfPatients, CountryList cL);