#ifndef JOBJ_H_INCLUDED
#define JOBJ_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../handling_error/herror.h"
#include "Jdata.h"

/* == Objet JSON == */

struct Jobj {

    int ID;

    /* Liste des données */
    Jdata *data;

};

typedef struct Jobj Jobj;

/* == Prototypes == */


Jobj* newJobj(void);
void addJobjData(Jobj *j, Jdata *data);

void deleteJobj(Jobj *j);

void debugJobj(Jobj *j, int lvl);

#endif // JOBJ_H_INCLUDED
