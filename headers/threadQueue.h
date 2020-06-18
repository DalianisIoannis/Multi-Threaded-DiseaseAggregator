#pragma once

#include "./general.h"
#include <pthread.h>

#define ISWORKER 1
#define ISQUERY 2

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
    int maxLength;
    qNodePtr head;
    qNodePtr tail;
} threadQueue;
typedef threadQueue* threadQueuePtr;


threadQueuePtr  newQueue();
void            enqueue(threadQueuePtr* Queue, int* clientSocket, int WhatWork);
void            delQueue(threadQueuePtr* Queue);
qNodePtr        dequeue(threadQueuePtr* Queue);
void            delThreadNode(qNodePtr* myNode);