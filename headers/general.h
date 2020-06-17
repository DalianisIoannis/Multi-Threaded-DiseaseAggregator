#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>

typedef struct str_node
{
    char *str;
    struct str_node *next;
} str_node;

typedef struct worker
{
    pid_t pid;
    int wfd;
    int rfd;
    str_node country;
} worker;

int compareDates(char*, char*);
bool isNumber(const char*);
int returnMaxInt(int, int);
void swapINT(int*, int*);
void swapSTR(char*, char*);
void selectionSort(int*, int, char**);