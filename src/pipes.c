#include "../headers/pipes.h"

int sendMessage(int fd, char* buf, int buf_size){
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

char* receiveMessage(int fd, char* buf, int buf_size){
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