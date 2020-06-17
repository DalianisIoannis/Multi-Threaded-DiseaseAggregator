#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include "./countryList.h"
#include "./workers.h"

int termination;
int sigkill;
int *pidAr;

int mySignalFlagForSIGINT_SIGQUIT;

void handler(int, siginfo_t*, void*);
void HandlerInit(struct sigaction*, void (*handler)(int, siginfo_t*, void*));
void handleSIGINTSIGQUIT(int signalVal, CountryList cL, requestStruct*);