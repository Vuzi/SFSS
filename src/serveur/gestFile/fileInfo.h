#ifndef FILEINFO_H_INCLUDED
#define FILEINFO_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../handling_error/herror.h"

/* == Structure d'un fichier == */

struct FileInfo {

    /* Nom du fichier */
    char* name;

    /* Nom de la racine */
    char* root;

    /* Poid du fichier */
    unsigned long long int size;

    /* Date de modification */
    int last_modif;
};

typedef struct FileInfo FileInfo;


/* == Prototypes == */

FileInfo* newFileInfo(char *name, char* root, unsigned long long int size, int last_modif);
void deleteFileInfo(FileInfo *f);

void debugFileInfo(FileInfo *f, int lvl);

#endif // FILEINFO_H_INCLUDED
