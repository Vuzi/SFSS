#ifndef STRTOOLS_H_INCLUDED
#define STRTOOLS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../handling_error/herror.h"

char *getStringPart(char* m, int start, int lenght);
char* getBracketContent(char** m, char open, char close);
bool isBlank(char c);
char* firstNonBlank(char *m);
char* getStringElem(char** m);
char* getFileContent(char* name);
char* copy(char* str);

#endif // STRTOOLS_H_INCLUDED
