#pragma once
#include "./general.h"

typedef struct cNode{
    char* country;
    struct cNode *next;
} cNode;
typedef cNode *countrylistNode;

typedef struct{
    countrylistNode front;
    countrylistNode rear;
} countryList;
typedef countryList *CountryList;


CountryList     initcountryList();
void            emptycountryList(CountryList*);
void            addCountryListNode(CountryList*, char*);
CountryList     makeCountryList(char*);
void            printCountryList(CountryList);