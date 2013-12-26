#ifndef FILELIST_H_INCLUDED
#define FILELIST_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../str/strTools.h"
#include "../handling_error/herror.h"

#include "fileInfo.h"

/* == Structure d'un fichier == */

struct FileList {

    /* ID du fichier (root + / + nom) */
    char* ID;

    /* Valeur (hash nom + date_modif + poid) */
    char* value;

    bool locked;

    /* Informations du fichier */
    struct FileInfo *info;

    /* Liste chainée */
    struct FileList *next;
    struct FileList *first;

};

typedef struct FileList FileList;


/* == Prototypes == */

FileList* getLastFileList(FileList *f);
FileList* getFirstFileList(FileList *f);

FileList* addFileList(FileList *last, char* ID, char* value);
FileList* newFileList(char *ID, char* value);

void deleteFileList(FileList *f);
void removeFileListFirst(FileList *list);
FileList* removeFileListElem(FileList *toRemove);

FileList* moveFileListElem(FileList *list, FileList *toAdd);
bool isInFileList(FileList *f, char* ID);
FileList* getFileListByID(FileList *list, char* ID);
unsigned long long int getTotalSizeFileList(FileList *f);

void debugFileList(FileList *f, int lvl);

#endif // FILELIST_H_INCLUDED
