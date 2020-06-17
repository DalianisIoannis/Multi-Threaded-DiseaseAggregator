#include "../headers/statistics.h"
#include "../headers/patients.h"

void freeStats(characteristic StN){
    if(StN!=NULL){
        free(StN->fileName);
        free(StN->country);
        free(StN->virusName);
        free(StN->age0_20);
        free(StN->age21_40);
        free(StN->age41_60);
        free(StN->age60);
    }
    free(StN);
}

characteristic initChar(){
    characteristic cc = malloc(sizeof(statStrings));
    cc->fileName = NULL;
    cc->country = NULL;
    cc->virusName = NULL;
    cc->age0_20 = NULL;
    cc->age21_40 = NULL;
    cc->age41_60 = NULL;
    cc->age60 = NULL;
    return cc;
}

StatisticsList initStatisticsList(){
    StatisticsList ll = malloc(sizeof(statList));
    if( ll==NULL ){
        fprintf(stderr, "Couldn't allocate Linked List. Abort...\n");
        return NULL;
    }
    (ll)->front     = NULL;
    (ll)->rear      = NULL;
    return ll;
}

void emptyStatisticsList(StatisticsList *ll) {

    StatsNode tmp;
    while ( (*ll)->front!=NULL ){
        freeStats((*ll)->front->item);

        tmp = (*ll)->front->next;
        
        (*ll)->front->next = NULL;
        free((*ll)->front);
        
        (*ll)->front = tmp;
    }
    free(*ll);
}

// gets a patientRecord and adds data on the characteristic list
void informCharList(StatisticsList* list, patientRecord pR){
    char* pRdis = strdup(pR->diseaseID);
    StatsNode tmp = (*list)->front;
    bool found=false;
    while ( tmp!=NULL && found==false){
        if( strcmp(pRdis, tmp->item->virusName)==0 && strcmp(pR->country, tmp->item->country)==0 && strcmp(pR->entryDate, tmp->item->fileName)==0){
            char* pRAge = strdup(pR->age);
            int intpRage = atoi(pRAge);
            // 
            if(intpRage<=20){
                char* lastNum = strdup(tmp->item->age0_20);
                int lastInt = atoi(lastNum)+1;
                free(tmp->item->age0_20);
                tmp->item->age0_20 = malloc(3*sizeof(char));
                sprintf(tmp->item->age0_20, "%d", lastInt);
                free(lastNum);
            }
            else if(intpRage<=40){
                char* lastNum = strdup(tmp->item->age21_40);
                int lastInt = atoi(lastNum)+1;
                free(tmp->item->age21_40);
                tmp->item->age21_40 = malloc(3*sizeof(char));
                sprintf(tmp->item->age21_40, "%d", lastInt);
                free(lastNum);
            }
            else if(intpRage<=60){
                char* lastNum = strdup(tmp->item->age41_60);
                int lastInt = atoi(lastNum)+1;
                free(tmp->item->age41_60);
                tmp->item->age41_60 = malloc(3*sizeof(char));
                sprintf(tmp->item->age41_60, "%d", lastInt);
                free(lastNum);
            }
            else{
                char* lastNum = strdup(tmp->item->age60);
                int lastInt = atoi(lastNum)+1;
                free(tmp->item->age60);
                tmp->item->age60 = malloc(3*sizeof(char));
                sprintf(tmp->item->age60, "%d", lastInt);
                free(lastNum);
            }
            free(pRAge);
            found=true;
            break;
        }
        tmp = tmp->next;
    }
    if(found==false){   // did not find same
        // only here input date country virus
        StatsNode llNode = malloc(sizeof(statNode));
    
        llNode->item = initChar();
        llNode->item->fileName = strdup(pR->entryDate);
        llNode->item->country = strdup(pR->country);
        llNode->item->virusName = strdup(pRdis);
        llNode->item->age0_20 = strdup("0");
        llNode->item->age21_40 = strdup("0");
        llNode->item->age41_60 = strdup("0");
        llNode->item->age60 = strdup("0");
        
        char* pRAge = strdup(pR->age);
        int intpRage = atoi(pRAge);
        if(intpRage<=20){
            free(llNode->item->age0_20);
            llNode->item->age0_20 = strdup("1");
        }
        else if(intpRage<=40){
            free(llNode->item->age21_40);
            llNode->item->age21_40 = strdup("1");
        }
        else if(intpRage<=60){
            free(llNode->item->age41_60);
            llNode->item->age41_60 = strdup("1");
        }
        else{
            free(llNode->item->age60);
            llNode->item->age60 = strdup("1");
        }

        if( (*list)->front==NULL ){   // empty list
            (*list)->front = llNode;
            (*list)->front->next = NULL;
            
            (*list)->rear = llNode;
            (*list)->rear->next = NULL;
        }
        else{
            (*list)->rear->next = llNode;
            (*list)->rear = llNode;
            (*list)->rear->next = NULL;
        }
        free(pRAge);
    }
    free(pRdis);
}

void printStat(characteristic cc){
    printf("fileName is %s\n", cc->fileName);
    printf("country is %s\n", cc->country);
    printf("Virus is %s\n", cc->virusName);
    printf("age0_20 is %s\n", cc->age0_20);
    printf("age21_40 is %s\n", cc->age21_40);
    printf("age41_60 is %s\n", cc->age41_60);
    printf("age60 is %s\n", cc->age60);
}

void printStatList(StatisticsList list){
    StatsNode tmp = list->front;
    while(tmp!=NULL) {
        printStat(tmp->item);
        tmp = tmp->next;
    }
}

char* concatStats(characteristic strChars){
    char* returner = malloc( (strlen(strChars->fileName)+1+strlen(strChars->country)+1+strlen(strChars->virusName)+1+strlen(strChars->age0_20)+1+strlen(strChars->age21_40)+1+strlen(strChars->age41_60)+1+strlen(strChars->age60)+1) );
    
    strcpy(returner, strChars->fileName);
    strcat(returner, " ");
    strcat(returner, strChars->country);
    strcat(returner, " ");
    strcat(returner, strChars->virusName);
    strcat(returner, " ");
    strcat(returner, strChars->age0_20);
    strcat(returner, " ");
    strcat(returner, strChars->age21_40);
    strcat(returner, " ");
    strcat(returner, strChars->age41_60);
    strcat(returner, " ");
    strcat(returner, strChars->age60);

    return returner;
}

void printStatsFromConcat(char* stat) {
    
    char* tok = strtok(stat, " ");
    printf("fileName is %s\n", tok);

    tok = strtok(NULL, " ");
    printf("country is %s\n", tok);

    tok = strtok(NULL, " ");
    printf("Virus is %s\n", tok);

    tok = strtok(NULL, " ");
    printf("age0_20 is %s\n", tok);

    tok = strtok(NULL, " ");
    printf("age21_40 is %s\n", tok);

    tok = strtok(NULL, " ");
    printf("age41_60 is %s\n", tok);

    tok = strtok(NULL, " ");
    printf("age60 is %s\n", tok);

}