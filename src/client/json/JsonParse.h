#ifndef JSONPARSE_H_INCLUDED
#define JSONPARSE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../handling_error/herror.h"
#include "Jobj.h"
#include "../str/strTools.h"

char* getNextJdataElemName(char** m);
Jdata* getNextJdataElem(Jdata *list, char **s);
Jobj* parseToJobj(char *s);

Jdata_type getType(char* data);

Jobj* fileToJobj(char* filename);

char* JobjToChar(Jobj *j);
void JobjToFile(Jobj *j, char* filename, char mode);

#endif // JSONPARSE_H_INCLUDED
