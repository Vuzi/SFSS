
#include "Jobj.h"

int JoobjIDtrack = -1;

/* =====================================================
                     addJobj

    last : éléments où ajouter

    Crée un objet J

   ===================================================== */
Jobj* newJobj(void) {

    Jobj *j = malloc(sizeof(Jobj));

    j->data = NULL;

    JoobjIDtrack++;
    j->ID = JoobjIDtrack;

    return j;
}

/* =====================================================
                     addJobjData

    j : éléments où ajouter
    data : liste de data à ajouter

    Ajoute data à l'objet JSON j, si l'objet j à déjà
    une data, celle-ci est supprimée.

   ===================================================== */
void addJobjData(Jobj *j, Jdata *data) {

    if(j->data) {
        /* Supprimer data existante */
        deleteJdataList(j->data);
    }

    j->data = data;
}

/* =====================================================
                     deleteJobj

    j : objet à supprimer

    Supprime la liste J et toutes ses données

   ===================================================== */
void deleteJobj(Jobj *j) {

    if(j) {

        /* Suppresion data */
        deleteJdataList(j->data);

        /* Suppresion liste */
        free(j);
    }
}

/* =====================================================
                        debugJobj

    j : liste à debuger
    lvl : nombre de tabulation

    Debug la liste J

   ===================================================== */
void debugJobj(Jobj *j, int lvl) {
    int i = 0;

    if(j) {
            for(i = 0; i<lvl; i++)
                printf("\t");
            printf("-Jobj (ID=%d) : \n", j->ID);

            if(j->data)
                debugJdata(j->data, lvl + 1);
    }
}



