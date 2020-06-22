#include "../headers/pipes.h"
#include "../headers/fatherFunctions.h"
#include "../headers/workers.h"
#include "../headers/signals.h"

int sendIPandPortToWorkers(workerDataNode* WorkerArray, int numWorkers, int bufferSize, char* serverIP, int serverPort) {
    
    char* strIP = strdup(serverIP);
    if(strIP==NULL) {
        perror("malloc");
        return -1;
    }
    
    char* strPort = malloc(12);
    if(strPort==NULL) {
        perror("malloc");
        return -1;
    }
    sprintf(strPort, "%d", serverPort);

    for (int i=0; i < numWorkers; i++) {
        sendMessage(WorkerArray[i]->fdWrite, strIP, bufferSize);
        sendMessage(WorkerArray[i]->fdWrite, strPort, bufferSize);
    }

    free(strIP);
    free(strPort);

    return 0;
}

int sendCountriesToWorkers(workerDataNode* WorkerArray, char* inDir, int numWorkers, int bufferSize, CountryList* ArForFather){

    struct dirent *entry;
    DIR* dir = opendir(inDir);

    if (ENOENT == errno) {
        fprintf(stderr, "Directory does not exist!\n");
        return -1;
    }
    else if (!dir) {
        fprintf(stderr, "opendir() failed for some other reason!\n");
        return -1;
    }

    int countNum=0;
    char* tmp;
    while ( (entry=readdir(dir))!=NULL )
    {
        if(strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0)
        {
            tmp = strdup(entry->d_name);
            if (tmp==NULL) {
                perror("MALLOC ERROR\n");
            }
            
            sendMessage(WorkerArray[countNum]->fdWrite, tmp, bufferSize);
            addCountryListNode(&(ArForFather[countNum]), tmp);
            
            free(tmp);
        }
        countNum++;
        if(countNum==numWorkers)
        {
            countNum=0;
        }
    }
    
    closedir(dir);
    
    for(int i=0; i<numWorkers; i++){
        sendMessage(WorkerArray[i]->fdWrite, "OK", bufferSize);
    }

    return 0;
}

int returnIforCountry(workerDataNode* WorkersArr, CountryList* WorkersCountries, char* country, int numWorkers) {

    for (int i=0; i < numWorkers; i++) {

        countrylistNode tmp = WorkersCountries[i]->front;
        while(tmp!=NULL){
            if(strcmp(country, tmp->country)==0) {
                return i;
            }
            tmp = tmp->next;
        }
    
    }
    return -1;
}