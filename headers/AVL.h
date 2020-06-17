#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "./patients.h"
#include "./general.h"

typedef struct AVLNode{
    patientRecord item;
    int     nodeHeight;
    char    *nodeKey;
    struct AVLNode *left;
    struct AVLNode *right;
} AVLNode;

typedef AVLNode *AVLNodePtr;


typedef struct{
    AVLNodePtr root;
} AVLTree;

typedef AVLTree *AVLTreePtr;



AVLTreePtr initAVLTree();
void    emptyAVLTree(AVLTreePtr);
void    emptyAVLnodes(AVLNodePtr);
bool    addAVLNode(AVLTreePtr, patientRecord, char*);
bool    compareAdd(AVLNodePtr*, AVLNodePtr*, char*);
void    printAVLTree(AVLTreePtr);
void    recPrintAVLNode(AVLNodePtr, int);
void    performRotations(AVLNodePtr*, AVLNodePtr*);
int     ReturnNodeHeight(AVLNodePtr);
void    get_child_nodes(AVLNodePtr, int*, char*, char*, char*);
AVLNodePtr  rotateNodeRight(AVLNodePtr);
AVLNodePtr  rotateNodeLeft(AVLNodePtr);
int     getBalanceFactor(AVLNodePtr);
void    RR_rotation(AVLNodePtr*);
void    LL_rotation(AVLNodePtr*);
void    RL_Rotation(AVLNodePtr*);
void    LR_Rotation(AVLNodePtr*);
int     getUnhealed(AVLNodePtr);
bool    UpdateExitDate(AVLNodePtr*, char*, char*);
void    get_exited_nodes(AVLNodePtr, int*, char*, char*, char*, char*);