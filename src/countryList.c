#include "../headers/countryList.h"

CountryList initcountryList()
{
    CountryList ll = malloc(sizeof(countryList));
    if( ll==NULL ){
        fprintf(stderr, "Couldn't allocate country List. Abort...\n");
        return NULL;
    }
    (ll)->front     = NULL;
    (ll)->rear      = NULL;
    return ll;
}

void emptycountryList(CountryList* cL)
{
    countrylistNode tmp;
    while ( (*cL)->front!=NULL ){
        free( (*cL)->front->country );

        tmp = (*cL)->front->next;
        
        (*cL)->front->next = NULL;
        free((*cL)->front);
        
        (*cL)->front = tmp;
    }
}

// for strtoking
CountryList makeCountryList(char* countries)
{
    CountryList cL = initcountryList();
    char* tmp = strdup(countries);
    char* tok = strtok(tmp, " ");
    tok = strtok(NULL, " ");
    while(tok!=NULL){
        addCountryListNode(&cL, tok);
        tok = strtok(NULL, " ");
    }
    free(tmp);
    return cL;
}

void addCountryListNode(CountryList* cL, char* country)
{
    countrylistNode node = malloc(sizeof(cNode));
    node->country = strdup(country);
    if( (*cL)->front==NULL ){   // empty list
        (*cL)->front = node;
        (*cL)->front->next = NULL;
        
        (*cL)->rear = node;
        (*cL)->rear->next = NULL;
    }
    else{
        (*cL)->rear->next = node;
        (*cL)->rear = node;
        (*cL)->rear->next = NULL;
    }
}

void printCountryList(CountryList ll)
{
    countrylistNode tmp = ll->front;
    while(tmp!=NULL){
        printf("\tCountry %s", tmp->country);
        tmp = tmp->next;
        printf("\n");
    }
}