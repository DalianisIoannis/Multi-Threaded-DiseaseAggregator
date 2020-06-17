#include "../headers/patients.h"

patientRecord initRecord(char* line, char* INdate, char* country){

    char *token;
    patientRecord p = malloc(sizeof(pRecord));
    if(p==NULL){ return NULL; }

    token = strtok(line," ");
    p->recordId = strdup(token);
    if(p->recordId==NULL){ return NULL; }

    token   = strtok(NULL, " ");

    token   = strtok(NULL, " ");
    p->patientFirstName = strdup(token);
    if(p->patientFirstName==NULL){ return NULL; }

    token   = strtok(NULL, " ");
    p->patientLastName = strdup(token);
    if(p->patientLastName==NULL){ return NULL; }

    token   = strtok(NULL, " ");
    p->diseaseID = strdup(token);
    if(p->diseaseID==NULL){ return NULL; }

    // token   = strtok(NULL, " \t\n");
    p->country = strdup(country);
    if(p->country==NULL){ return NULL; }

    // token   = strtok(NULL, " \t\n");
    p->entryDate = strdup(INdate);
    if(p->entryDate==NULL){ return NULL; }

    token   = strtok(NULL, "\n");
    p->age = strdup(token);
    if(p->age==NULL){ return NULL; }

    // p->exitDate=NULL;
    // token   = strtok(NULL, " \t\n");
    p->exitDate = strdup("--");
    if(p->exitDate==NULL){ return NULL; }

    return p;
}

void printRecord(patientRecord p){
    printf("Patient with recordId \t%s:\n", p->recordId);
    printf("\tpatientFirstName \t%s\n", p->patientFirstName);
    printf("\tpatientLastName \t%s\n", p->patientLastName);
    printf("\tdiseaseID \t%s\n", p->diseaseID);
    printf("\tcountry \t%s\n", p->country);
    printf("\tentryDate \t%s\n", p->entryDate);
    printf("\tage \t%s\n", p->age);
    
    printf("\texitDate \t%s\n", p->exitDate);
}

void deleteRecord(patientRecord* p){
    free( (*p)->recordId );
    free( (*p)->patientFirstName );
    free( (*p)->patientLastName );
    free( (*p)->diseaseID );
    free( (*p)->country );
    free( (*p)->entryDate );
    free( (*p)->exitDate );
    free( (*p)->age );
    free( *p );
}

void changePatientExitDate(patientRecord *p, char *date) {
    free( (*p)->exitDate );
    (*p)->exitDate = strdup(date);
}