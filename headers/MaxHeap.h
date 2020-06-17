#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "./general.h"

typedef struct {

    char* age0_20;
    char* age21_40;
    char* age41_60;
    char* age61;

} agesRange;
typedef agesRange* agesRangePtr;

agesRangePtr initAgeRangeStruct();
void    freeagesRangePtr(agesRangePtr*);
void    uploadRangeStats(agesRangePtr*, char*, int);
char*   printRange(agesRangePtr* node, int k);

typedef struct HeapNode{
    char    *occurence;
    int     total;
    int     id;
    struct HeapNode *right;
    struct HeapNode *left;
} HeapNode;

typedef struct HeapNode *HeapNodePtr;

typedef struct{
    HeapNodePtr root;
} MaxHeap;

typedef MaxHeap *MaxHeapPtr;


MaxHeapPtr initMaxHeap();
void emptyMaxHeap(MaxHeapPtr);
void emptyMaxHeapNode(HeapNodePtr);
void printMaxHeapNode(HeapNodePtr, int);
bool addMaxHeapNode(MaxHeapPtr, char*, int*);
bool searchAllexisting(HeapNodePtr*, char*);
void findParentOfInsertedNode(HeapNodePtr, int, HeapNodePtr*);
void insertNodetoMaxHeap(HeapNodePtr*, HeapNodePtr);
void reheapify(HeapNodePtr*);
char* printKlargestItems(MaxHeapPtr, int, int*);
HeapNodePtr getNodeWithId(HeapNodePtr, int, HeapNodePtr*);