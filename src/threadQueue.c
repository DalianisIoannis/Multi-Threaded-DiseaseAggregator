#include "../headers/threadQueue.h"

threadQueuePtr newQueue(int ss) {
    // cond_nonempty = PTHREAD_COND_INITIALIZER;
    // cond_nonfull = PTHREAD_COND_INITIALIZER;

    threadQueuePtr tQueue = malloc(sizeof(threadQueue));

    tQueue->bufferSize = ss;
    tQueue->head = NULL;
    tQueue->tail = NULL;

    tQueue->start = 0;
    tQueue->end = -1;
    tQueue->count = 0;
    tQueue->threadAr = malloc((tQueue->bufferSize)*sizeof(qNodePtr));
    for(int i=0; i<tQueue->bufferSize; i++) {
    //     ((tQueue->threadAr)[i]) = malloc(sizeof(qNode));
        ((tQueue->threadAr)[i]) = malloc(sizeof(qNodePtr));
    }
    // tQueue->threadAr = malloc((tQueue->bufferSize)*sizeof(qNode));

    return tQueue;
}

void enqueue(threadQueuePtr* Queue, int* clientSocket, int WhatWork) {

    pthread_mutex_lock(&mtx);

    // // // // // // // // // // // // // // // // //
    // // // // // // // // // // // // // // // // //
    // while((*Queue)->count >= (*Queue)->bufferSize) {
    //     // found full
    //     pthread_cond_wait(&cond_nonfull, &mtx);
    // }
    // // // // // // // // // // // // // // // // //
    // // // // // // // // // // // // // // // // //

    (*Queue)->end = ((*Queue)->end + 1) % ((*Queue)->bufferSize);

    qNodePtr newNode = malloc(sizeof(qNode));
    newNode->qSocket = *clientSocket;
    newNode->port = NULL;
    newNode->identity = WhatWork;
    newNode->next = NULL;
    
    ((*Queue)->threadAr)[(*Queue)->end] = newNode;
    (*Queue)->count++;

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

    pthread_mutex_unlock(&mtx);

}

qNodePtr dequeue(threadQueuePtr* Queue) {

    // // // // // // // // // // // // // // // // //
    // // // // // // // // // // // // // // // // //
    // qNodePtr ret = NULL;
    // pthread_mutex_lock(&mtx);
    // // while((*Queue)->count <= 0) {
    // //     // empty buffer
    // //     pthread_cond_wait(&cond_nonempty, &mtx);
    // // }

    // ret = (*Queue)->threadAr[(*Queue)->start];
    // // // // // // // // // // // // // // // // //
    // // // // // // // // // // // // // // // // //
    (*Queue)->start = ((*Queue)->start+1) % (*Queue)->bufferSize;
    (*Queue)->count--;

    if((*Queue)->head==NULL) {
        pthread_mutex_unlock(&mtx);
        return NULL;
    }
    else {
        qNodePtr ret = (*Queue)->head;
        (*Queue)->head = (*Queue)->head->next;
        if((*Queue)->head==NULL) {
           (*Queue)->tail = NULL; 
        }
        pthread_mutex_unlock(&mtx);
        return ret;
    }
    pthread_mutex_unlock(&mtx);
    // return ret;
    return NULL;
}

void delThreadNode(qNodePtr* myNode) {

    if((*myNode)->port!=NULL) {
        free((*myNode)->port);
    }
    
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