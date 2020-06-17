#include "../headers/linkedList.h"

Linked_List initlinkedList(){
    Linked_List ll = malloc(sizeof(lList));
    if( ll==NULL ){
        fprintf(stderr, "Couldn't allocate Linked List. Abort...\n");
        return NULL;
    }
    (ll)->front     = NULL;
    (ll)->rear      = NULL;
    return ll;
}

bool addNode(Linked_List *ll, patientRecord pR){
    listNode llNode = malloc(sizeof(lNode));
    if(llNode==NULL){ return false; }
    
    llNode->item = pR;

    if( (*ll)->front==NULL ){   // empty list
        (*ll)->front = llNode;
        (*ll)->front->next = NULL;
        
        (*ll)->rear = llNode;
        (*ll)->rear->next = NULL;
    }
    else{
        (*ll)->rear->next = llNode;
        (*ll)->rear = llNode;
        (*ll)->rear->next = NULL;
    }
    return true;
}

void emptyLinkedList(Linked_List *ll){

    listNode tmp;
    while ( (*ll)->front!=NULL ){
        deleteRecord( &((*ll)->front->item) );

        tmp = (*ll)->front->next;
        
        (*ll)->front->next = NULL;
        free((*ll)->front);
        
        (*ll)->front = tmp;
    }
}

void printLinkedList(Linked_List ll){
    listNode tmp = ll->front;
    while(tmp!=NULL){
        printRecord(tmp->item);
        tmp = tmp->next;
        printf("\n");
    }
}

bool updateExitDate(const Linked_List *ll, patientRecord pR){
    listNode tmp = (*ll)->front;
    while(tmp!=NULL){
        // if( strcmp(tmp->item->recordId, pR->recordId)==0 && strcmp(tmp->item->country, pR->country)==0  && strcmp(tmp->item->diseaseID, pR->diseaseID)==0 && strcmp(tmp->item->patientLastName, pR->patientLastName)==0 ){
        if( strcmp(tmp->item->recordId, pR->recordId)==0 ){

            int comparer = compareDates(tmp->item->entryDate, pR->entryDate);
            if( comparer==0 || comparer==2 ){
                changePatientExitDate( &(tmp->item), pR->entryDate );
            }
            else{
                // printf("Can't have exited on that date!\n");
                return false;
            }
            return true;
        }
        tmp = tmp->next;
    }
    return false;
}

char* returnPatientifExists(Linked_List ll, char* ID) {

    listNode tmp = ll->front;
    while(tmp!=NULL){
        if( strcmp(tmp->item->recordId, ID)==0 ){

            char* returner = malloc( (strlen(tmp->item->recordId)+strlen(tmp->item->patientFirstName)+strlen(tmp->item->patientLastName)+strlen(tmp->item->diseaseID)+strlen(tmp->item->entryDate)+strlen(tmp->item->exitDate)+strlen(tmp->item->age)+7+1)*sizeof(char) );
            strcpy(returner, tmp->item->recordId);
            strcat(returner, " ");
            strcat(returner, tmp->item->patientFirstName);
            strcat(returner, " ");
            strcat(returner, tmp->item->patientLastName);
            strcat(returner, " ");
            strcat(returner, tmp->item->diseaseID);
            strcat(returner, " ");
            strcat(returner, tmp->item->age);
            strcat(returner, " ");
            strcat(returner, tmp->item->entryDate);
            strcat(returner, " ");
            strcat(returner, tmp->item->exitDate);
            return returner;

        }
        tmp = tmp->next;
    }
    return NULL;
}