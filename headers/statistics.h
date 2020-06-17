#pragma once

#include "./general.h"
#include "./patients.h"

#include <stdbool.h>

typedef struct statStrings{
    char* fileName;
    char* country;
    char* virusName;
    char* age0_20;
    char* age21_40;
    char* age41_60;
    char* age60;
} statStrings;
typedef statStrings *characteristic;

typedef struct statNode{
    characteristic item;
    struct statNode *next;
}statNode;
typedef statNode *StatsNode;

typedef struct statList{
    StatsNode front;
    StatsNode rear;
}statList;
typedef statList *StatisticsList;

void freeStats(characteristic);
StatisticsList initStatisticsList();
characteristic initChar();
void emptyStatisticsList(StatisticsList*);
void informCharList(StatisticsList*, patientRecord);
void printStat(characteristic);
void printStatList(StatisticsList);
char* concatStats(characteristic);
void printStatsFromConcat(char*);