#ifndef JDATA_H_INCLUDED
#define JDATA_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../handling_error/herror.h"

/* == Différents types de données == */

enum Jdata_type {
    NB_INT = 1, NB_FLOAT = 2, STRING = 3, JSON = 4, NONE = 0
};

typedef enum Jdata_type Jdata_type;


/* == Liste de valeur d'un élément JSON */

struct Jdata {

    int ID;

    /* Nom de la valeur */
    char* name;

    /* Type et valeur */
    Jdata_type type;
    union Jdata_value *d;

    /* Liste chainée */
    struct Jdata *next;
    struct Jdata *first;

};

typedef struct Jdata Jdata;


/* == Union de la valeur en elle même == */

union Jdata_value {
    unsigned long long int i;
    float f;
    char* s;
    struct Jobj* j;
};

typedef union Jdata_value Jdata_value;


/* == Prototypes == */

#include "Jobj.h"

Jdata* getLastJdata(Jdata *j);
Jdata* getFirstJdata(Jdata *j);

Jdata* newJdata(char *name);
Jdata* addJdata(Jdata *last, char *name, Jdata_type type, void* data);
Jdata* addJdataString(Jdata *last, char *name, char* data);
Jdata* addJdataInt(Jdata *last, char *name, unsigned long long int data);
void addJdataData(Jdata *j, Jdata_type type, void* data);
void addJdataDataString(Jdata *j, char* data);
Jdata* addJdataJson(Jdata *last, char *name, struct Jobj *j);

Jdata* getNextJdataByName(Jdata *j, char* name);
Jdata* getNextJdataByNameAndType(Jdata *j, char* name, Jdata_type type);
Jdata* getNextJdataStringByName(Jdata *j, char* name);
Jdata* getNextJdataJsonByName(Jdata *j, char* name);
Jdata* getNextJdataIntByName(Jdata *j, char* name);
Jdata* getNextJdataFloatByName(Jdata *j, char* name);

char* getAndCopyJdataString(Jdata *j, char* strname);

void deleteJdataList(Jdata *j);
Jdata* removeLastJdata(Jdata *j);

void debugJdata(Jdata *j, int lvl);

#endif // JDATA_H_INCLUDED
