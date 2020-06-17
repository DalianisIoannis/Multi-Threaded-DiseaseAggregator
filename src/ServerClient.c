#include "../headers/pipes.h"
#include "../headers/general.h"
#include "../headers/ServerClient.h"


char* bin2hex(const unsigned char *input, size_t len) {
    char* result;
    char* hexits = "0123456789ABCDEF";

    if(input==NULL || len<=0) {
        return NULL;
    }

    int resultlength = (len*3)+1;

    result = malloc(resultlength);
    bzero(result, resultlength);

    for(int i=0; i<len; i++) {
        result[i*3] = hexits[input[i] >> 4];
        result[(i*3)+1] = hexits[input[i] & 0x0F];
        result[(i*3)+2] = ' ';
    }
    return result;
}


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

// void printTry(const int newsock) {

//     char buf[256] = {0};

//     read(newsock, buf, strlen("geia"));

//     printf("Gave %s\n", buf);

//     printf("Closing  connection .\n");
//     close(newsock);
// }


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