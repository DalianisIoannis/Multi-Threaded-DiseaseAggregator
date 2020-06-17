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

    // sprintf(strIP, "%d", serverIP);
    
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

void FatherQuerries(workerDataNode* WorkersArr, int numWorkers, int bufferSize, CountryList* WorkersCountries) {

    char *inputString = NULL, *buffer = NULL;
    char *tmp;
    size_t size = 0;

    printf("Select instruction.\n");

    while( getline(&buffer, &size, stdin)!=-1 ){

        if( strcmp(buffer, "\n")!=0 ){
            inputString = strtok(buffer, "\n");
        }
        else{ free(buffer); }

        if( inputString==NULL ){ tmp = strdup("$$$"); }
        else{ tmp = strdup(inputString); }
        
        if(tmp==NULL){
            printf("String failure.\n");
            return;
        }

        if(strcmp(tmp, "/exit")==0) {
            printf("exiting\n");
            break;
        }
        else if(strcmp(tmp, "/listCountries")==0) {
    
            for(int i=0; i<numWorkers; i++) {

                sendMessage(WorkersArr[i]->fdWrite, tmp, bufferSize);
                // read countries with pid
                for( ; ; ) {

                    char arr[bufferSize];
                    char* readed = receiveMessage(WorkersArr[i]->fdRead, arr, bufferSize);

                    if(strcmp(readed, "OK")==0){
                        free(readed);
                        break;
                    }

                    printf("%s\n", readed);
                    free(readed);
                }
            }
        }
        else {

            char* dupString = strdup(tmp);
            
            char* instruct = strtok(dupString," ");
            char* ind1 = strtok(NULL," ");
            char* ind2 = strtok(NULL," ");
            char* ind3 = strtok(NULL," ");
            char* ind4 = strtok(NULL," ");

            if( strcmp(instruct, "/diseaseFrequency")==0 ){ // diseaseFrequency virusName date1 date2 [country]

                if( ind1==NULL || ind2==NULL || ind3==NULL ){
                    printf("Need to provide proper variables.\n\n");
                }
                else{
                    int compDat = compareDates(ind2, ind3);
                    if(compDat!=0 && compDat!=2){
                        printf("Give correct dates.\n\n");
                    }
                    else{
                        if( ind4==NULL ){ // didn't give country
                            
                            int totalOccs = 0;
                            for(int i=0; i<numWorkers; i++) {

                                sendMessage(WorkersArr[i]->fdWrite, tmp, bufferSize);
                                
                                char arr[bufferSize];
                                char* readed = receiveMessage(WorkersArr[i]->fdRead, arr, bufferSize);
                                totalOccs=totalOccs+atoi(readed);
                                
                                free(readed);
                            }
                            printf("Total occs %d\n", totalOccs);

                        }
                        else{
                            
                            int position = returnIforCountry(WorkersArr, WorkersCountries, ind4, numWorkers);
                            
                            if(position!=-1) {
                            // for(int i=0; i<numWorkers; i++) {
                                
                                sendMessage(WorkersArr[position]->fdWrite, tmp, bufferSize);
                                
                                char arr[bufferSize];
                                char* readed = receiveMessage(WorkersArr[position]->fdRead, arr, bufferSize);                                
                                if(atoi(readed)!=0) {
                                    printf("%d\n", atoi(readed));
                                }
                                free(readed);
                            
                            // }
                            }
                        }
                    }
                }
            }
            else if(strcmp(instruct, "/topk-AgeRanges")==0) {

                int position = returnIforCountry(WorkersArr, WorkersCountries, ind2, numWorkers);
                if(position!=-1) {
                // for(int i=0; i<numWorkers; i++) {
                    
                    sendMessage(WorkersArr[position]->fdWrite, tmp, bufferSize);

                    char arr[bufferSize];
                    char* readed = receiveMessage(WorkersArr[position]->fdRead, arr, bufferSize);
                    if( strcmp(readed, "WRONG")!=0 ) {
                        printf("%s\n", readed);
                    }
                    free(readed);

                // }
                }
            }
            else if(strcmp(instruct, "/searchPatientRecord")==0) {

                for(int i=0; i<numWorkers; i++) {
                    
                    sendMessage(WorkersArr[i]->fdWrite, tmp, bufferSize);
                    
                    char arr[bufferSize];
                    char* readed = receiveMessage(WorkersArr[i]->fdRead, arr, bufferSize);
                    if( strcmp(readed, "WRONG")!=0 ) {
                        printf("from child: %s\n", readed);
                    }
                    free(readed);

                }

            }
            else if(strcmp(instruct, "/numPatientAdmissions")==0) {
                if(ind4!=NULL) {
                    int position = returnIforCountry(WorkersArr, WorkersCountries, ind4, numWorkers);
                    if(position!=-1) {
                        sendMessage(WorkersArr[position]->fdWrite, tmp, bufferSize);

                        for( ; ; ) {
                            char arr[bufferSize];
                            char* readed = receiveMessage(WorkersArr[position]->fdRead, arr, bufferSize);


                            if(strcmp(readed, "OK")==0){
                                free(readed);
                                break;
                            }

                            printf("%s\n", readed);
                            
                            free(readed);
                        }
                    }
                }
                else {
                    for(int i=0; i<numWorkers; i++) {

                        sendMessage(WorkersArr[i]->fdWrite, tmp, bufferSize);

                        for( ; ; ) {
                            char arr[bufferSize];
                            char* readed = receiveMessage(WorkersArr[i]->fdRead, arr, bufferSize);


                            if(strcmp(readed, "OK")==0){
                                free(readed);
                                break;
                            }

                            printf("%s\n", readed);
                            
                            free(readed);
                        }

                    }
                }
            }
            else if(strcmp(instruct, "/numPatientDischarges")==0) {
                if(ind4==NULL) {
                    for(int i=0; i<numWorkers; i++) {
                        
                        sendMessage(WorkersArr[i]->fdWrite, tmp, bufferSize);

                        for( ; ; ) {

                            char arr[bufferSize];
                            char* readed = receiveMessage(WorkersArr[i]->fdRead, arr, bufferSize);

                            if(strcmp(readed, "OK")==0){
                                free(readed);
                                break;
                            }

                            printf("%s\n", readed);
                            
                            free(readed);

                        }
                    }
                }
                else {
                    int position = returnIforCountry(WorkersArr, WorkersCountries, ind4, numWorkers);
                    // for(int i=0; i<numWorkers; i++) {
                    if(position!=-1) {
                        sendMessage(WorkersArr[position]->fdWrite, tmp, bufferSize);
                        
                        char arr[bufferSize];
                        char* readed = receiveMessage(WorkersArr[position]->fdRead, arr, bufferSize);;

                        if(strcmp(readed, "NONE")!=0){
                            printf("%s\n", readed);
                        }
                        
                        free(readed);
                    // }
                    }
                }
            }

            free(dupString);
        }

        free(tmp);
        tmp = NULL;
        free(inputString);
        inputString = NULL;
        buffer = NULL;
        printf("Select instruction.\n");
    }
    free(tmp);
    free(inputString);
    buffer = NULL;

    for(int i=0; i<numWorkers; i++) {
        sendMessage(WorkersArr[i]->fdWrite, "OK", bufferSize);
    }

}