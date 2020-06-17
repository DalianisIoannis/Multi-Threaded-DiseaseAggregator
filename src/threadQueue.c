#include "../headers/threadQueue.h"

void f() {
    printf("GIAGIA\n");
}

threadQueuePtr newQueue() {
    threadQueuePtr tQueue = malloc(sizeof(threadQueue));

    tQueue->head = NULL;
    tQueue->tail = NULL;
    return tQueue;
}

void enqueue(threadQueuePtr* Queue, int* clientSocket) {
    qNodePtr newNode = malloc(sizeof(qNode));

    newNode->qSocket = *clientSocket;
    newNode->port = NULL;
    newNode->next = NULL;

    if((*Queue)->tail == NULL) {
        (*Queue)->tail = newNode;
        (*Queue)->tail->next = NULL;

        (*Queue)->head = newNode;
        (*Queue)->head->next = NULL;
    }
    else {
        (*Queue)->tail->next = newNode;
        (*Queue)->tail = newNode;
        (*Queue)->tail->next = NULL;
    }
}

qNodePtr dequeue(threadQueuePtr* Queue) {
    if((*Queue)->head==NULL) {
        return NULL;
    }
    else {
        qNodePtr ret = (*Queue)->head;
        (*Queue)->head = (*Queue)->head->next;
        if((*Queue)->head==NULL) {
           (*Queue)->tail = NULL; 
        }
        return ret;
    }
    return NULL;
}

void delThreadNode(qNodePtr* myNode) {

    if((*myNode)->port!=NULL) {
        free((*myNode)->port);
    }
    // if((*myNode)->clientSocket!=NULL) {
    //     free((*myNode)->clientSocket);
    // }
    free(*myNode);

}

void delQueue(threadQueuePtr* Queue) {

    qNodePtr tmp=((*Queue)->head);

    while(tmp!=NULL) {
        qNodePtr toDel = tmp;

        tmp = tmp->next;

        delThreadNode(&toDel);
    }

    free(*Queue);
}