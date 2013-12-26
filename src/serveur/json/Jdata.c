
#include "Jdata.h"

/* =====================================================
                     getLastJdata

    name : liste d'éléments à traiter

    Renvoit le dernier élément d'une liste, ou NULL si
    celle-ci est nulle

   ===================================================== */
Jdata* getLastJdata(Jdata *j) {
    while(j && j->next) {
        j = j->next;
    }
    return j;
}

/* =====================================================
                     getFirstJdata

    name : liste d'éléments à traiter

    Renvoit le premier élément d'une liste, ou NULL si
    celle-ci est nulle

   ===================================================== */
Jdata* getFirstJdata(Jdata *j) {
    if(j)
        return j->first;
    else
        return NULL;
}

/* =====================================================
                     addJdata

    last : liste d'éléments où ajouter
    name : nom du nouvel élément
    type : type de la donnée
    data : pointeur sur donnée

    Ajoute un élément à la liste last. Si last est null,
    alors une nouvelle liste sera crée. La doinnée sera
    ajoutée. Pour créer une donnée vide, utiliser le type
    "NONE"

   ===================================================== */
Jdata* addJdata(Jdata *last, char *name, Jdata_type type, void* data) {

    Jdata *j = malloc(sizeof(Jdata));

    j->name = name;
    j->type = NONE;
    j->d = NULL;
    j->next = NULL;

    /* Nouvelle liste */
    if (!last) {
        j->ID = 0;
        j->first = j;
    }
    /* Ajout à une liste */
    else {
        last = getLastJdata(last);
        j->ID = last->ID + 1;
        j->first = last->first;
        last->next = j;
    }

    /* Données */
    addJdataData(j, type, data);

    return j;
}

Jdata* addJdataString(Jdata *last, char *name, char* data) {
    return addJdata(last, name, STRING, (void*)data);
}

Jdata* addJdataInt(Jdata *last, char *name, unsigned long long int data) {
    return addJdata(last, name, NB_INT, (void*)(&data));
}

Jdata* addJdataJson(Jdata *last, char *name, Jobj *j) {
    return addJdata(last, name, JSON, (void*)(j));
}

/* =====================================================
                     addJdataData

    j : liste Jdata
    type : type de la donnée
    data : pointeur sur donnée

    Ajoute un élément data à l'objet J, et supprime celui
    existant

   ===================================================== */
void addJdataData(Jdata *j, Jdata_type type, void* data) {

    if(j) {

        /* Si il y a des data on les efface */
        if(j->type != NONE) {
            if(j->type == JSON)
                deleteJobj(j->d->j);
            else if(j->type == STRING)
                free(j->d->s);

            free(j->d);
        }

        /* récupération du type & valeur */
        j->type = type;

        if(type == NONE) {
            j->type = NONE;
            j->d = NULL;
        } else {
            j->d = malloc(sizeof(Jdata_value));

            if (type == NB_INT) {
                j->d->i = *((unsigned long long int*)(data));
            } else if (type == NB_FLOAT) {
                j->d->f = *((float*)(data));
            } else if (type == STRING) {
                j->d->s = (char*)(data);
            } else if (type == JSON) {
                if(data != NULL)
                    j->d->j = (Jobj*)data;
                else {
                    HERROR_WRITE(HERROR_ERROR, "[JSON] JSON vide");
                    free(j->d);
                    j->type = NONE;
                    j->d = NULL;
                }
            } else {
                HERROR_WRITE(HERROR_ERROR, "[JSON] Type inconnu");
                free(j->d);
                j->type = NONE;
                j->d = NULL;
            }
        }
    }
}

void addJdataDataString(Jdata *j, char* data) {
    addJdataData(j, STRING, (void*)data);
}

/* =====================================================
                     newJdata

    name : nom du nouvel élément

    Crée une nouvelle liste avec comme nom de premier
    élement name

   ===================================================== */
Jdata* newJdata(char *name) {
    return addJdata(NULL, name, NONE, NULL);
}

/* =====================================================
                     getNextJdataByName

    j : liste où cherche
    name : nom à chercher

    Renvoit la première occurence de name trouvée.
    Commence à j et non pas à j->first. Renvoit NULL s'il
    ne le trouve pas.

   ===================================================== */
Jdata* getNextJdataByName(Jdata *j, char* name) {
    while(j) {
        if(strcmp(j->name, name) == 0)
            return j;
        else
            j = j->next;
    }
    return NULL;
}

Jdata* getNextJdataByNameAndType(Jdata *j, char* name, Jdata_type type) {
    while(j) {
        if(strcmp(j->name, name) == 0 && j->type == type)
            return j;
        else
            j = j->next;
    }
    return NULL;
}

Jdata* getNextJdataStringByName(Jdata *j, char* name) {
    return getNextJdataByNameAndType(j, name, STRING);
}

Jdata* getNextJdataJsonByName(Jdata *j, char* name) {
    return getNextJdataByNameAndType(j, name, JSON);
}

Jdata* getNextJdataIntByName(Jdata *j, char* name) {
    return getNextJdataByNameAndType(j, name, NB_INT);
}

Jdata* getNextJdataFloatByName(Jdata *j, char* name) {
    return getNextJdataByNameAndType(j, name, NB_FLOAT);
}

char* getAndCopyJdataString(Jdata *j, char* strname) {

    char *str = NULL;

    if(j && strname) {
        j = getNextJdataStringByName(j->first, strname);

        if(j) {
            str = malloc(sizeof(char)*(strlen(j->d->s)+1));
            strcpy(str, j->d->s);
        } else {
            HERROR_WRITE(HERROR_ERROR, "[Json] La liste n'est pas correctement formatee, impossible de la traduire (Un element ne comporte pas d'attribut '%s')", strname);
        }
    }

    return str;
}

/* =====================================================
                     deleteJdataList

    j : liste à supprimer

    Supprime la liste J et tout les élément qu'elle
    contient.

   ===================================================== */
void deleteJdataList(Jdata *j) {

    Jdata* tmp;

    if(j) {
        j = j->first;
        tmp = j;

        /* Pour chaque liste... */
        while(tmp) {
            j = tmp;
            tmp = j->next;

            /* Suppression data */
            if(j->type != NONE) {
                if(j->type == JSON && j->d->j)
                    deleteJobj(j->d->j);
                else if(j->type == STRING && j->d->s)
                    free(j->d->s);

                free(j->d);
            }

            /* Suppresion nom */
            free(j->name);

            /* Suppresion liste */
            free(j);
        }
    }
}

/* =====================================================
                     removeLastJdata

    j : liste où supprimer les denier élément

    Supprime le dernier élément de la liste j et renvoit
    le premier ou NULL si la liste était où est devenue
    vide.

   ===================================================== */
Jdata* removeLastJdata(Jdata *j) {

    if(j) {
        Jdata* last = getLastJdata(j);
        j = j->first;

        if(last == j->first) {
            j = NULL;
        } else {
           while(j && j->next->next) {
                j = j->next;
            }
            j->next = NULL;
            j = j->first;
        }

        /* Suppression data */
        if(last->type != NONE) {
            if(last->type == JSON)
                deleteJobj(last->d->j);
            else if(last->type == STRING)
                free(last->d->s);

            free(last->d);
        }

        /* Suppresion nom */
        free(last->name);

        /* Suppresion liste */
        free(last);
    }

    return j;
}

/* =====================================================
                        debugJdata

    j : liste à debuger
    lvl : nombre de tabulation

    Debug la liste J

   ===================================================== */
void debugJdata(Jdata *j, int lvl) {
    int i = 0;

    if(j) {
        j = j->first;

        while(j) {
            for(i = 0; i<lvl; i++)
                printf("\t");
            printf("-Jdata (ID=%d) : %s", j->ID, j->name);

            if(j->type == NONE)
                printf(" = (NONE)\n");
            else if (j->type == STRING)
                printf(" = %s (STRING)\n", j->d->s);
            else if (j->type == NB_INT)
                printf(" = %lld (INT)\n", j->d->i);
            else if (j->type == NB_FLOAT)
                printf(" = %f (FLOAT)\n", j->d->f);
            else if (j->type == JSON) {
                printf(" (JSON)\n");
                debugJobj(j->d->j, lvl+1);
            }

            j = j->next;
        }
    }
}

