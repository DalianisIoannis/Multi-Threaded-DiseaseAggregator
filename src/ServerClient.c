#include "../headers/pipes.h"
#include "../headers/general.h"
#include "../headers/ServerClient.h"

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


void handleConnection(int client_socket) {
    char buffer[BUFSIZE];
    size_t bytes_read;
    int msgsize = 0;
    char actualpath[_PC_PATH_MAX+1];

    while((bytes_read = read(client_socket, buffer+msgsize, sizeof(buffer)-msgsize-1)) > 0) {
        msgsize += bytes_read;
        if(msgsize>BUFSIZE-1 || buffer[msgsize-1] == '\n') {
            break;
        }
    }

    check(bytes_read, "recv error.\n");

    buffer[msgsize-1] = 0;

    printf("Request: %s\n", buffer);

    fflush(stdout);

    if(realpath(buffer, actualpath) == NULL) {
        printf("ERROR WITH PATH: %s\n", buffer);
        close(client_socket);
        return;
    }

    FILE *fp = fopen(actualpath, "r");
    if(fp==NULL) {
        printf("ERROR WITH OPEN: %s\n", buffer);
        close(client_socket);
        return;
    }

    while( (bytes_read = fread(buffer, 1, BUFSIZE, fp)) > 0) {
        printf("sending %zu bytes\n", bytes_read);
        write(client_socket, buffer, bytes_read);
    }

    close(client_socket);
    fclose(fp);

    printf("Closing connection\n");
}


void child_serverNoThread(int newsock) {
    char  buf[1];
    while(read(newsock , buf , 1) > 0) {
        /*  Receive  1 char  */
        
        putchar(buf [0]);
        /*  Print  received  char  */
        /*  Capitalize  character  */
        buf[0] = toupper(buf [0]);
        
        /*  Reply  */
        if (write(newsock , buf , 1) < 0) {
            perror_exit("write");
        }
    }
    printf("Closing  connection .\n");
    close(newsock);
    /*  Close  socket  */
}


void *child_serverThread(void *Pnewsock) {
    int newsock = *((int*)Pnewsock);
    free(Pnewsock);
    char  buf[1];
    while(read(newsock , buf , 1) > 0) {
        /*  Receive  1 char  */
        
        putchar(buf [0]);
        /*  Print  received  char  */
        /*  Capitalize  character  */
        buf[0] = toupper(buf [0]);
        
        /*  Reply  */
        if (write(newsock , buf , 1) < 0) {
            perror_exit("write");
        }
    }
    printf("Closing  connection .\n");
    close(newsock);
    /*  Close  socket  */
    return NULL;
}

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

void inputTofirstEmpty(WorkersInfo* ar, char* pidPort, char* address) {

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

        // (((*ar)->myWorkers[i])->serverptr)

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

        // sendMessageSock((((*ar)->myWorkers[i])->sock), "pou");

        close((((*ar)->myWorkers[i])->sock));
    }
}