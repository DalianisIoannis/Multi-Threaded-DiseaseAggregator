#pragma once

#include "./patients.h"
#include "./general.h"

#include <stdbool.h>

typedef struct lNode{
    patientRecord item;
    struct lNode *next;
} lNode;

typedef lNode *listNode;


typedef struct{
    listNode front;
    listNode rear;
}lList;

typedef lList *Linked_List;


Linked_List initlinkedList();
bool    addNode(Linked_List*, patientRecord);
void    emptyLinkedList(Linked_List*);
void    printLinkedList(Linked_List);
bool    updateExitDate(const Linked_List*, patientRecord);
char*   returnPatientifExists(Linked_List, char*);