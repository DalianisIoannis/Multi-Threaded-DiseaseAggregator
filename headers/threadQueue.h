#pragma once

#include "./general.h"
#include <pthread.h>

#define ISWORKER 1
#define ISQUERY 2

pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;

int totalInserts;

typedef struct qNode {

    int *clientSocket;
    int qSocket;
    int identity;   // 0 for empty 1 for worker 2 for query
    char* port;

    struct qNode *next;
} qNode;
typedef qNode* qNodePtr;

typedef struct {
    int lenngth;
    int bufferSize;

    qNodePtr* threadAr; // bufferSize
    int start;
    int end;
    int count;

    qNodePtr head;
    qNodePtr tail;
} threadQueue;
typedef threadQueue* threadQueuePtr;


threadQueuePtr  newQueue(int);
void            enqueue(threadQueuePtr* Queue, int* clientSocket, int WhatWork);
void            delQueue(threadQueuePtr* Queue);
qNodePtr        dequeue(threadQueuePtr* Queue);
void            delThreadNode(qNodePtr* myNode);