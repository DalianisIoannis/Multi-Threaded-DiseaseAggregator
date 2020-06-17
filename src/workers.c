#include "../headers/workers.h"
#include "../headers/countryList.h"
#include "../headers/general.h"
#include "../headers/pipes.h"
#include "../headers/statistics.h"
#include "../headers/linkedList.h"
#include "../headers/patients.h"
#include "../headers/statistics.h"
#include "../headers/HashTable.h"
#include "../headers/signals.h"

void printWorkerNode(workerDataNode node){
    printf("Worker Node:\n");
    printCountryList(node->PIDcountries);
    printf("\tpid %d\n", node->pid);
    printf("\tfifoRead %s\n", node->fifoRead);
    printf("\tfifoWrite %s\n", node->fifoWrite);
    printf("\ttotalCountries %d\n", node->totalCountries);
    printf("\tfdRead %d\n", node->fdRead);
    printf("\tfdWrite %d\n", node->fdWrite);
}

workerDataNode makeWorkerArCell(pid_t pid, int bufferSize){
    
    workerDataNode cell = malloc(sizeof(workerData));
    
    if(cell==NULL){ return NULL; }
    
    cell->pid = pid;
    
    cell->fifoRead = malloc( (strlen("./pipeFiles/reader_des")+12) );
    sprintf((cell)->fifoRead, "./pipeFiles/reader_des%d", pid);
    if (mkfifo(cell->fifoRead, 0666) == -1) {
        perror("mkfifor");
        return NULL;
    }

    (cell)->fifoWrite = malloc( (strlen("./pipeFiles/writer_des")+12) );
    sprintf((cell)->fifoWrite, "./pipeFiles/writer_des%d", pid);
    if (mkfifo(cell->fifoWrite, 0666) == -1) {
        perror("mkfifo");
        return NULL;
    }

    if ((cell->fdRead = open(cell->fifoRead, O_RDONLY)) == -1) {
        perror("open");
    }
    
    if ((cell->fdWrite = open(cell->fifoWrite, O_WRONLY)) == -1) {
        perror("open");

    }

    cell->IPaddres = NULL;
    cell->PortNumber = NULL;

    // take serverIP and serverPort
    // char arr[bufferSize];
    // char* readed;
    // readed = receiveMessage(cell->fdRead, arr, bufferSize);
    // node->IPaddres = strdup(readed);
    // node->IPaddres = malloc(10);
    // printf("Got %s\n", readed);
    // free(readed);
    
    // readed = receiveMessage(rfd, arr, bufferSize);
    // node->PortNumber = strdup(readed);
    // free(readed);

    return cell;
}

void emptyworkerNode(workerDataNode *wL){
    
    free((*wL)->IPaddres);

    free((*wL)->PortNumber);

    free((*wL)->fifoRead);

    free((*wL)->fifoWrite);
    
    free(*wL);

}

int WorkerRun(char* inDir, int bufferSize, int rfd, int wfd){

    requestStruct* reQS = malloc(sizeof(requestStruct));
    reQS->TOTAL=0;
    reQS->SUCCESS=0;

    char arr[bufferSize];
    struct dirent *entry;
    char* readed;
    char* Ipaddres;
    int ServerPort;

    CountryList cL = initcountryList();
    characteristic charsList = initChar();
    Linked_List ListOfPatients = initlinkedList();
    Linked_List ListOfEXITPatients = initlinkedList();
    CountryList listOfDateFiles = initcountryList();


    // take serverIP and serverPort
    readed = receiveMessage(rfd, arr, bufferSize);
    Ipaddres = strdup(readed);
    free(readed);
    
    readed = receiveMessage(rfd, arr, bufferSize);
    ServerPort = atoi(readed);
    free(readed);

    printf("Ipaddres %s and ServerPort %d\n", Ipaddres, ServerPort);

    for( ; ; ){ // read countries
        
        readed = receiveMessage(rfd, arr, bufferSize);

        if(strcmp(readed, "OK")==0){
            free(readed);
            break;
        }
        
        // addCountryListNode(&(node->PIDcountries), readed);
        addCountryListNode(&(cL), readed);

        free(readed);
    }

    // for countries take patients
    countrylistNode tmpNode = cL->front;

    StatisticsList charsForDateFile = initStatisticsList();

    while(tmpNode!=NULL) {
        
        char* CountryDir = malloc( (strlen(inDir)+strlen(tmpNode->country)+1)*sizeof(char) );
        strcpy(CountryDir, inDir);
        strcat(CountryDir, tmpNode->country);
        
        DIR* dirr = opendir(CountryDir);
        
        if (ENOENT == errno) {
            fprintf(stderr, "Directory does not exist!\n");
            return -1;
        }
        else if (!dirr) {
            fprintf(stderr, "opendir() failed for some other reason!\n");
            return -1;
        }

        while( (entry=readdir(dirr))!=NULL ) {
            if(strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0) {
            
                // StatisticsList charsForDateFile = initStatisticsList();

                char* countryDateFile = malloc( (strlen(CountryDir)+strlen(entry->d_name)+1+1)*sizeof(char) );
                strcpy(countryDateFile, CountryDir);
                strcat(countryDateFile, "/");
                strcat(countryDateFile, entry->d_name);

                // printf("%s\n", countryDateFile);
                addCountryListNode(&(listOfDateFiles), countryDateFile);

                FILE *fp = fopen(countryDateFile, "r");
                if(fp==NULL) {
                    perror("FILE.");
                }
                
                char* buffer = NULL;
                size_t size = 0;
                while( getline(&buffer, &size, fp)>=0 ) {

                    inputPatientsToStructures(buffer, &(ListOfPatients), entry->d_name, tmpNode->country, &charsForDateFile, &ListOfEXITPatients);  // every line ends with /n
                    
                    free(buffer);
                    buffer = NULL;
                }


                free(countryDateFile);
                free(buffer);
                fclose(fp);
            
            }
        }

        free(CountryDir);
        closedir(dirr);
        tmpNode = tmpNode->next;
    }
    

    // check if EXITS exist in ENTRIES
    listNode looker = ListOfEXITPatients->front;
    while(looker!=NULL) {

        if(updateExitDate(&ListOfPatients, looker->item)==false) {
            printf("ERROR\n");
        }

        looker = looker->next;
    }

    // input to Data Structures
    int diseaseHashtableNumOfEntries = 8;
    int countryHashtableNumOfEntries = 3;
    int bucketSize = 24;

    HashTable   HT_disease, HT_country;
    if( (HT_disease = initHashTable(bucketSize, diseaseHashtableNumOfEntries))==NULL ){
        fprintf(stderr, "Couldn't allocate Hash Table. Abort...\n");
        return false;
    }

    inputLLtoHT(ListOfPatients, HT_disease, 0);    // 0 for disease

    if( (HT_country = initHashTable(bucketSize, countryHashtableNumOfEntries))==NULL ){
        fprintf(stderr, "Couldn't allocate Hash Table. Abort...\n");
        return false;
    }

    inputLLtoHT(ListOfPatients, HT_country, 1);    // 1 for country

    // printf("Hash Table of Countries is:\n"); printHashTable(HT_country);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // communicate with server
    struct sockaddr_in servaddr;
    int sockfd, n;
    int sendbytes;
    char sendline[4096];
    char recvline[4096];

    int sock;
    char buf[256];
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct  sockaddr *)&server;
    struct hostent *rem;


    if ((sock = socket(AF_INET , SOCK_STREAM , 0)) < 0) {
        perror_exit("socket");
    }    
    
    /*  Find  server  address  */
    if ((rem = gethostbyname(Ipaddres)) == NULL) {
        herror("gethostbyname");
        exit (1);
    }

    server.sin_family = AF_INET; /*  Internet  domain  */
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(ServerPort);/*  Server  port  */


    /*  Initiate  connection  */
    if (connect(sock , serverptr , sizeof(server)) < 0) {
        perror_exit("connect");
    }

    memset(&server, 0, sizeof(struct sockaddr_in));
    socklen_t sa_len = sizeof(struct sockaddr_in);
    int tmpRealPort = getsockname(sock, (struct sockaddr *)&server, &sa_len);
    printf("In pid %d returned %d\n", getpid(), tmpRealPort);
    char* toNtoa = strdup(inet_ntoa(server.sin_addr));
    printf("In pid %d toNtoa %s\n", getpid(), toNtoa);
    free(toNtoa);
    int ntoHs = ntohs(server.sin_port);
    printf("In pid %d ntoHs %d\n", getpid(), ntoHs);

    
    printf("Connecting to %s port %d\n", Ipaddres, ServerPort);

    // write(sock , "geia", strlen("geia"));
    sendMessageSock(sock, "geia");



    // send statistics
    StatsNode tmpStatsNode = charsForDateFile->front;
    while(tmpStatsNode!=NULL) {

        char* stringStats = concatStats(tmpStatsNode->item);
        // sendMessage(wfd, stringStats, bufferSize);
        sendMessageSock(sock, stringStats);
        free(stringStats);

        tmpStatsNode = tmpStatsNode->next;
    }
    emptyStatisticsList(&charsForDateFile);

    // sendMessage(wfd, "OK", bufferSize);
    sendMessageSock(sock, "OK");

    close(sock);/*  Close  socket  and  exit  */

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////

    // read from Father Querries
    for( ; ; ) { // read querries
        
        if(mySignalFlagForSIGINT_SIGQUIT==-1) {
            handleSIGINTSIGQUIT(mySignalFlagForSIGINT_SIGQUIT, cL, reQS);
            mySignalFlagForSIGINT_SIGQUIT=0;
        }

        if(mySignalFlagForSIGINT_SIGQUIT==-2) {
            // read new folder
            StatisticsList charsForDateFile = initStatisticsList();
            countrylistNode tmpNode = cL->front;
            while(tmpNode!=NULL) {
                char* CountryDir = malloc( (strlen(inDir)+strlen(tmpNode->country)+1)*sizeof(char) );
                strcpy(CountryDir, inDir);
                strcat(CountryDir, tmpNode->country);
                
                DIR* dirr = opendir(CountryDir);
                
                if (ENOENT == errno) {
                    fprintf(stderr, "Directory does not exist!\n");
                    return -1;
                }
                else if (!dirr) {
                    fprintf(stderr, "opendir() failed for some other reason!\n");
                    return -1;
                }

                while( (entry=readdir(dirr))!=NULL ) {
                    if(strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0) {
                        char* countryDateFile = malloc( (strlen(CountryDir)+strlen(entry->d_name)+1+1)*sizeof(char) );
                        strcpy(countryDateFile, CountryDir);
                        strcat(countryDateFile, "/");
                        strcat(countryDateFile, entry->d_name);

                        // printf("%s\n", countryDateFile);

                        int found=0;
                        countrylistNode tmp = listOfDateFiles->front;
                        while(tmp!=NULL){
                            if(strcmp(tmp->country, countryDateFile)==0) {
                                found=1;
                            }
                            tmp = tmp->next;
                        }
                        if(found==0) {
                            printf("NEW %s\n", countryDateFile);
                            addCountryListNode(&(listOfDateFiles), countryDateFile);
                            FILE *fp = fopen(countryDateFile, "r");
                            if(fp==NULL) {
                                perror("FILE.");
                            }
                            char* buffer = NULL;
                            size_t size = 0;
                            while( getline(&buffer, &size, fp)>=0 ) {
                                // inputPatientsToStructures(buffer, &(ListOfPatients), entry->d_name, tmpNode->country, &charsForDateFile, &ListOfEXITPatients);  // every line ends with /n
                                free(buffer);
                                buffer = NULL;
                            }
                            fclose(fp);
                            free(buffer);
                        }
                        free(countryDateFile);
                    }
                }
                free(CountryDir);
                closedir(dirr);
                tmpNode = tmpNode->next;
            }
            emptyStatisticsList(&charsForDateFile);
            mySignalFlagForSIGINT_SIGQUIT=0;
        }

        readed = receiveMessage(rfd, arr, bufferSize);
        

        if(strcmp(readed, "OK")==0){
            free(readed);
            break;
        }
        else {
            reQS->SUCCESS = reQS->SUCCESS+1;
            reQS->TOTAL = reQS->TOTAL+1;
        }
        
        if(strcmp(readed, "/listCountries")==0) {
            // print countries with pid

            countrylistNode node = cL->front;
            while(node!=NULL) {
                
                char* tmp = malloc( (strlen(node->country)+12+1) );
                strcpy(tmp, node->country);
                strcat(tmp, " ");
                char* tmpPIDstring = malloc( 12 );
                sprintf(tmpPIDstring, "%d", getpid());
                strcat(tmp, tmpPIDstring);

                sendMessage(wfd, tmp, bufferSize);

                free(tmp);
                free(tmpPIDstring);
                node = node->next;

            }

            sendMessage(wfd, "OK", bufferSize);

        }
        else {
         
            char* instruct = strtok(readed," ");
            char* ind1 = strtok(NULL," ");
            char* ind2 = strtok(NULL," ");
            char* ind3 = strtok(NULL," ");
            char* ind4 = strtok(NULL," ");
            char* ind5 = strtok(NULL," ");

            if( strcmp(instruct, "/diseaseFrequency")==0 ){ // diseaseFrequency virusName date1 date2 [country]

                if( ind4==NULL ){ // didn't give country
                    
                    int res = diseaseFrequencyNoCountry(HT_disease, ind1, ind2, ind3);
                    char* newInt = malloc(12);
                    sprintf(newInt, "%d", res);
                    
                    sendMessage(wfd, newInt, bufferSize);
                    free(newInt);

                }
                else{
                    
                    int res = diseaseFrequencyCountry(HT_country, ind1, ind4, ind2, ind3);
                    char* newInt = malloc(12);
                    sprintf(newInt, "%d", res);
                    sendMessage(wfd, newInt, bufferSize);
                    free(newInt);

                }

            }
            else if(strcmp(instruct, "/topk-AgeRanges")==0) {
                if( ind5==NULL ){
                    printf("Need to provide proper variables.\n");
                    sendMessage(wfd, "WRONG", bufferSize);
                }
                else {
                    
                    char *returned = topkAgeRanges(HT_country, ind1, ind2, ind3, ind4, ind5);

                    if(returned!=NULL) {
                        sendMessage(wfd, returned, bufferSize);
                    }
                    else {
                        sendMessage(wfd, "WRONG", bufferSize);
                    }

                    free(returned);
                    
                }
            }
            else if(strcmp(instruct, "/searchPatientRecord")==0) {

                char* receive = returnPatientifExists(ListOfPatients, ind1);
                if(receive!=NULL) {
                    sendMessage(wfd, receive, bufferSize);
                }
                else {
                    sendMessage(wfd, "WRONG", bufferSize);
                }
                free(receive);

            }
            else if(strcmp(instruct, "/numPatientAdmissions")==0) {

                    if(ind4==NULL) {    // no country
                        countrylistNode node = cL->front;
                        while(node!=NULL) {
                            
                            int received = diseaseFrequencyCountry(HT_country, ind1, node->country, ind2, ind3);
                            
                            char* intTostr = malloc(12);
                            sprintf(intTostr, "%d", received);

                            char* strCatToRet = malloc(strlen(node->country)+strlen(intTostr)+1+1);
                            strcpy(strCatToRet, node->country);
                            strcat(strCatToRet, " ");
                            strcat(strCatToRet, intTostr);

                            sendMessage(wfd, strCatToRet, bufferSize);
                            
                            free(intTostr);
                            free(strCatToRet);
                            node = node->next;
                        }
                        sendMessage(wfd, "OK", bufferSize);
                    }
                    else {
                        int received = diseaseFrequencyCountry(HT_country, ind1, ind4, ind2, ind3);
                            
                        if(received!=0) {
                            char* intTostr = malloc(12);
                            sprintf(intTostr, "%d", received);

                            char* strCatToRet = malloc(strlen(ind4)+strlen(intTostr)+1+1);
                            strcpy(strCatToRet, ind4);
                            strcat(strCatToRet, " ");
                            strcat(strCatToRet, intTostr);

                            sendMessage(wfd, strCatToRet, bufferSize);
                            
                            free(intTostr);
                            free(strCatToRet);
                        }

                        sendMessage(wfd, "OK", bufferSize);

                    }

            }
            else if(strcmp(instruct, "/numPatientDischarges")==0) {
                if(ind4==NULL) {    // no country
                    countrylistNode node = cL->front;
                    while(node!=NULL) {
                        char* tmp =numPatientDischargesCountry(HT_country, ind1, node->country, ind2, ind3);
                        // printf("Country %s %s\n", node->country,  tmp);
                        
                        char* wholeStr = malloc( (strlen(node->country)+2+strlen(tmp))*sizeof(char) );
                        strcpy(wholeStr, node->country);
                        strcat(wholeStr, " ");
                        strcat(wholeStr, tmp);

                        sendMessage(wfd, wholeStr, bufferSize);

                        free(wholeStr);
                        free(tmp);
                        node = node->next;
                    }
                    sendMessage(wfd, "OK", bufferSize);
                }
                else {

                    char* tmp =numPatientDischargesCountry(HT_country, ind1, ind4, ind2, ind3);

                    if(strcmp(tmp, "0")==0) {
                        sendMessage(wfd, "NONE", bufferSize);
                        free(tmp);
                    }
                    else {
                        char* wholeStr = malloc( (strlen(ind4)+2+strlen(tmp))*sizeof(char) );
                        strcpy(wholeStr, ind4);
                        strcat(wholeStr, " ");
                        strcat(wholeStr, tmp);
                        sendMessage(wfd, wholeStr, bufferSize);
                        free(wholeStr);
                        free(tmp);
                    }
                }
            }
        
        }

        free(readed);
    }

    deleteHT(HT_disease);
    deleteHT(HT_country);
    
    freeStats(charsList);

    emptyLinkedList(&ListOfEXITPatients);
    free(ListOfEXITPatients);

    emptyLinkedList(&ListOfPatients);
    free(ListOfPatients);

    emptycountryList(&listOfDateFiles);
    free(listOfDateFiles);

    emptycountryList(&cL);
    free(cL);

    free(Ipaddres);

    free(reQS);
    return 0;
}

int inputPatientsToStructures(char* line, Linked_List *ListOfPatients, char* date, char* country, StatisticsList* charsForDateFile, Linked_List *ListOfEXITPatients) {

    char* tmp = strdup(line);
    char* tok = strtok(tmp, " ");
    tok = strtok(NULL, " ");

    patientRecord pR = initRecord(line, date, country);

    if(strcmp(tok, "ENTER")==0) {
        
        addNode(ListOfPatients, pR);

        informCharList(charsForDateFile, pR);

    }
    else {

        addNode(ListOfEXITPatients, pR);

    }

    free(tmp);
    return 0;
}