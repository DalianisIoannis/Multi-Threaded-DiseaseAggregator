#include "../headers/pipes.h"
#include "../headers/general.h"
#include "../headers/ServerClient.h"
#include "../headers/countryList.h"

void err_n_die(const char* fmt, ...) {
    int errno_save = errno;
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    fflush(stdout);

    if(errno_save!=0) {
        fprintf(stdout, "(errno = %d) : %s\n", errno_save, strerror(errno_save));
        fprintf(stdout, "\n");
        fflush(stdout);
    }
    
    va_end(ap);
    exit(1);
}


int check(int exp, const char* msg) {
    if(exp == SOCKETERROR) {
        perror(msg);
        exit(1);
    }
    return exp;
}


//     fflush(stdout);


/*  Wait  for  all  dead  child  processes  */
void  sigchld_handler (int  sig) {
    while (waitpid(-1, NULL , WNOHANG) > 0);
}


void  perror_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}


int sendMessageSock(int fd, char* buf) {

    int buf_size = 100;
    char str[12] = {0};
    int stSize = sizeof(str);
    int bufLen = strlen(buf)+1;

    sprintf(str, "%u", bufLen);

    for(int i=0; i<stSize/buf_size; i++) {
        write(fd, str+(i*buf_size), buf_size);
    }
    if(stSize%buf_size!=0) {
        write(fd, str+(stSize-stSize%buf_size), stSize%buf_size);
    }

    for(int i=0; i<bufLen/buf_size; i++) {
        write(fd, buf+(i*buf_size), buf_size);
    }

    if(bufLen%buf_size!=0) {
        write(fd, buf+(bufLen-bufLen%buf_size), bufLen%buf_size);
    }
    return 0;    
}

char* receiveMessageSock(int fd, char* buf) {
    int buf_size = 100;
    char* return_buf = NULL;
    char str[12] = {0};
    int stSize = sizeof(str);
    int return_bufSize = 0;
    
    for(int i=0; i<stSize/buf_size; i++) {
        read(fd, buf, buf_size);
        memcpy(str+(i*buf_size), buf, buf_size);
    }
    if(stSize%buf_size!=0) {
        read(fd, buf, stSize%buf_size);
        memcpy(str+(stSize/buf_size)*buf_size, buf, stSize-(stSize/buf_size)*buf_size);
    }
    return_bufSize = (int)strtol(str, NULL, 10);
    if( (return_buf=malloc(return_bufSize))==NULL ) {
        printf("malloc ERROR\n");
        return NULL;
    }
    for(int i=0; i<return_bufSize/buf_size; i++) {
        read(fd, buf, buf_size);
        memcpy(return_buf+(i*buf_size), buf, buf_size);
    }

    if(return_bufSize%buf_size!=0) {
        read(fd, buf, return_bufSize%buf_size);
        memcpy(return_buf+(return_bufSize/buf_size)*buf_size, buf, return_bufSize-(return_bufSize/buf_size)*buf_size);
    }

    return return_buf;
}

void inputTofirstEmpty(WorkersInfo* ar, char* pidPort, char* address, int port) {

    char arr[1024];
    if( (*ar)->hasAcceptedFirst==false ) {
        ((*ar)->hasAcceptedFirst) = true;
    }

    for(int i=0; i<((*ar)->numOfworkers); i++) {
        if( (((*ar)->myWorkers[i])->hasBeenSet)==false ) {
            (((*ar)->myWorkers[i])->Ipaddr) = strdup(address);
            (((*ar)->myWorkers[i])->hasBeenSet) = true;
            int getPID = atoi(pidPort);
            (((*ar)->myWorkers[i])->pidOfWorker) = getPID;
            char* tmpPort = malloc( (strlen("1")+12) );
            sprintf(tmpPort, "1%d", getPID);
            (((*ar)->myWorkers[i])->portNum) = atoi(tmpPort);
            free(tmpPort);

            // accept countries from workers
            for( ; ; ){ // read countries
        
                char *readed = receiveMessageSock(port, arr);

                if(strcmp(readed, "OK")==0){
                    free(readed);
                    break;
                }
                
                printf("Received %s\n", readed);
                addCountryListNode(&((((*ar)->myWorkers[i])->countriesOfWorker)), readed);

                free(readed);
            }

            break;
        }
    }
    return;
}

void printWorkerInfo(WorkersInfo ar) {

    if(ar!=NULL) {
        if(ar->hasAcceptedFirst!=false) {
            // int numOfWork = ar->numOfworkers;
            printf("hasBeenMade is %d and numOfworkers %d\n", ar->hasBeenMade, ar->numOfworkers);
            for(int i=0; i<ar->numOfworkers; i++) {
                // if( (ar->myWorkers)[i] )
                printf("For place %d hasBeenSet is %d pidOfWorker is %d and portNum is %d\n", i, (ar->myWorkers)[i]->hasBeenSet, (ar->myWorkers)[i]->pidOfWorker, (ar->myWorkers)[i]->portNum);
            }
        }
    }

}

void connectToallWorkers(WorkersInfo* ar) {
    if(ar==NULL) {
        return;
    }

    for(int i=0; i<((*ar)->numOfworkers); i++) {

        (((*ar)->myWorkers[i])->serverptr) = (struct  sockaddr *)&(((*ar)->myWorkers[i])->server);

        if (( (((*ar)->myWorkers[i])->sock) = socket(AF_INET , SOCK_STREAM , 0)) < 0) {
            perror_exit("socket");
        }    

        if (((((*ar)->myWorkers[i])->rem) = gethostbyname((((*ar)->myWorkers[i])->Ipaddr))) == NULL) {
            herror("gethostbyname");
            exit (1);
        }

        (((*ar)->myWorkers[i])->server).sin_family = AF_INET; /*  Internet  domain  */
        memcpy(&(((*ar)->myWorkers[i])->server).sin_addr, (((*ar)->myWorkers[i])->rem)->h_addr, (((*ar)->myWorkers[i])->rem)->h_length);
        (((*ar)->myWorkers[i])->server).sin_port = htons((((*ar)->myWorkers[i])->portNum));/*  Server  port  */

        if (connect((((*ar)->myWorkers[i])->sock) , (((*ar)->myWorkers[i])->serverptr) , sizeof((((*ar)->myWorkers[i])->server))) < 0) {
            perror_exit("connect");
        }

    }
}

void sendMsgToAllWorkers(WorkersInfo* ar, char* msg) {

    char arr[1024];

    if(ar==NULL) {
        return;
    }

    for(int i=0; i<((*ar)->numOfworkers); i++) {

        sendMessageSock((((*ar)->myWorkers[i])->sock), msg);

        char* readed = receiveMessageSock((((*ar)->myWorkers[i])->sock), arr);
        printf("From Master to Server %s\n", readed);
        free(readed);

    }
}

void FinishallWorkers(WorkersInfo* ar) {
    if(ar==NULL) {
        return;
    }

    for(int i=0; i<((*ar)->numOfworkers); i++) {

        sendMessageSock((((*ar)->myWorkers[i])->sock), "ENDOFQUERRIES");

        close((((*ar)->myWorkers[i])->sock));
    }
}