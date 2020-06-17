#include "../headers/signals.h"
#include "../headers/general.h"
#include "../headers/workers.h"
#include "../headers/countryList.h"

void HandlerInit(struct sigaction *act, void (*handler)(int, siginfo_t*, void*)) {

    act->sa_handler = (void*)handler;

    act->sa_sigaction = handler;
    
    sigfillset(&act->sa_mask);
    
    act->sa_flags = SA_NODEFER | SA_RESTART | SA_SIGINFO;
    
    // sigaction(SIGCHLD, act, NULL);
    // sigaction(SIGPIPE, act, NULL);
    sigaction(SIGUSR1, act, NULL);
    sigaction(SIGINT, act, NULL);
    sigaction(SIGQUIT, act, NULL);
}

void handler(int sig, siginfo_t* siginfo, void* buf) {
    if(sig==SIGINT || sig==SIGQUIT) {
        printf("Child %d caught SIGINT or SIGQUIT.\n", getpid());
        mySignalFlagForSIGINT_SIGQUIT=-1;
    }
    else if(sig==SIGUSR1) {
        printf("Child %d caught SIGUSR1.\n", getpid());
        mySignalFlagForSIGINT_SIGQUIT=-2;
    }
}

void handleSIGINTSIGQUIT(int signalVal, CountryList cL, requestStruct* rQS) {

    if(signalVal==-1) {
        printf("mySignalFlagForSIGINT_SIGQUIT in worker %d is %d\n", getpid(), signalVal);
        FILE* fptr;

        char* filName = malloc((strlen("log_file."))+12);
        strcpy(filName, "log_file.");
        char* tmpNum = malloc(12);
        sprintf(tmpNum, "%d", getpid());
        strcat(filName, tmpNum);

        free(tmpNum);

        char* dir = malloc( strlen("./logFiles/")+12+strlen("log_file.") );
        strcpy(dir, "./logFiles/");
        strcat(dir, filName);

        free(filName);

        fptr = fopen(dir, "w");

        if(fptr==NULL) {
            perror("create File.");
        }

        free(dir);
        
        countrylistNode tmp = cL->front;
        while(tmp!=NULL){
            fprintf(fptr, "%s\n", tmp->country);
            tmp = tmp->next;
        }
        fprintf(fptr, "\nTOTAL: %d\n", rQS->TOTAL);
        fprintf(fptr, "SUCCESS: %d\n", rQS->SUCCESS);

        fclose(fptr);
    }
}