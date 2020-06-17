#pragma once

#include "./general.h"
#include <pthread.h>


typedef struct qNode {

    int *clientSocket;
    int qSocket;
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

void f();
threadQueuePtr newQueue();
void enqueue(threadQueuePtr* Queue, int* clientSocket);
void delQueue(threadQueuePtr* Queue);
qNodePtr dequeue(threadQueuePtr* Queue);
void delThreadNode(qNodePtr* myNode);