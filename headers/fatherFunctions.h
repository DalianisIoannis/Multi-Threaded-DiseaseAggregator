#pragma once

#include "./countryList.h"
#include "./general.h"
#include "./workers.h"

int     sendCountriesToWorkers(workerDataNode*, char* inDir, int, int, CountryList*);
void    FatherQuerries(workerDataNode*, int, int, CountryList*);
int     returnIforCountry(workerDataNode*, CountryList*, char*, int);
int     sendIPandPortToWorkers(workerDataNode*, int, int, char*, int);