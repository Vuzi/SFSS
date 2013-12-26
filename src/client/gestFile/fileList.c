
#include "fileList.h"

/* =====================================================
                     getLastFileList

    name : liste d'éléments à traiter

    Renvoit le dernier élément d'une liste, ou NULL si
    celle-ci est nulle.

   ===================================================== */
FileList* getLastFileList(FileList *f) {
    while(f && f->next) {
        f = f->next;
    }
    return f;
}


/* =====================================================
                     getFirstFileList

    name : liste d'éléments à traiter

    Renvoit le premier élément d'une liste, ou NULL si
    celle-ci est nulle.

   ===================================================== */
FileList* getFirstFileList(FileList *f) {
    if(f)
        return f->first;
    else
        return NULL;
}


/* =====================================================
                     addFileList

    Ajout un élément à la liste last.

   ===================================================== */
FileList* addFileList(FileList *last, char* ID, char* value){

    FileList *f = malloc(sizeof(FileList));

    f->ID = ID;
    f->value = value;
    f->locked = false;
    f->info = NULL;

    f->next = NULL;

    /* Nouvelle liste */
    if (!last) {
        f->first = f;
    }
    /* Ajout à une liste */
    else {
        last = getLastFileList(last);
        f->first = last->first;
        last->next = f;
    }

    return f;
}


/* =====================================================
                     newFileList

    Crée une nouvelle liste.

   ===================================================== */
FileList* newFileList(char *ID, char* value) {
    return addFileList(NULL, ID, value);
}


/* =====================================================
                     deleteFileList

    f : liste à supprimer

    Supprime la liste f et tout les élément qu'elle
    contient.

   ===================================================== */
void deleteFileList(FileList *f) {

    FileList* tmp;

    if(f) {
        f = f->first;
        tmp = f;

        /* Pour chaque liste... */
        while(tmp) {
            f = tmp;
            tmp = f->next;

            /* Suppression nom */
            free(f->ID);

            /* Suppression root */
            free(f->value);

            /* Suppression info */
            deleteFileInfo(f->info);

            free(f);
        }
    }
}


void removeFileListFirst(FileList *list) {
    FileList *tmp = NULL, *first = NULL;

    if(list) {
        first = tmp = list->first;

        while(tmp) {
            tmp->first = first;
            tmp = tmp->next;
        }

        list->next = NULL;

        deleteFileList(list);
    }
}


bool isInFileList(FileList *f, char* ID) {
    while(f) {
        if(!strcmp(f->ID, ID))
            return true;
        f = f->next;
    }
    return false;
}


FileList* getFileListByID(FileList *list, char* ID) {

    if(list && ID) {
        list = list->first;

        while(list) {
            if(strcmp(list->ID, ID) == 0)
                return list;
            list = list->next;
        }

    }
    return NULL;
}

FileList* removeFileListElem(FileList *toRemove) {

    FileList *after = NULL, *before = NULL, *first = NULL, *tmp = NULL;

    if(toRemove) {

        first = toRemove->first;
        after = toRemove->next;

        if(toRemove->first == toRemove)
            before = NULL;
        else {
            before = first;

            while(before->next != toRemove)
                before = before->next;
        }

        /* Si pas premier */
        if(before && after) {
            before->next = after;

            tmp = before->first;
        }
        /* Si juste avant */
        else if(before) {
            before->next = NULL;

            tmp = before->first;
        }
        /* Si juste après */
        else if(after) {
            tmp = after;
            while(tmp) {
                tmp->first = after;
                tmp = tmp->next;
            }

            tmp = after;
        }
        /* Plus rien */
        else
            tmp = NULL;

        toRemove->next = NULL;
        toRemove->first = toRemove;
        deleteFileList(toRemove);

    }

    return tmp;
}

FileList* moveFileListElem(FileList *list, FileList *toAdd) {
    FileList *tmp = NULL;

    if(toAdd) {
        if(list) {
            /* Le fichier existe, on le modifie */
            if((tmp = getFileListByID(list, toAdd->ID))) {

                /* Valeur */
                free(tmp->value);
                tmp->value = copy(toAdd->value);

                /* Info */
                deleteFileInfo(tmp->info);
                tmp->info = toAdd->info;
                toAdd->info = NULL;

                /* Supp. l'ancien fichier */
                deleteFileList(toAdd);

                tmp->locked = false;

            }
            /* On l'ajoute */
            else {
                getLastFileList(list)->next = toAdd;
                toAdd->first = list->first;
            }
        } else {
            toAdd->first = toAdd;
            list = toAdd;
        }
    }

    return list;
}


unsigned long long int getTotalSizeFileList(FileList *f) {

    unsigned long long int r = 0;

    if(f) {
        f = f->first;

        while(f) {
            r += f->info->size;
            f = f->next;
        }
    }

    return r;
}

/* =====================================================
                        debugFileList

    f : liste à debuger
    lvl : nombre de tabulation

    Debug la liste f.

   ===================================================== */
void debugFileList(FileList *f, int lvl) {
    int i = 0;

    if(f) {
        f = f->first;

        while(f) {
            putchar('\n');

            for(i = 0; i<lvl; i++)
                putchar('\t');

            printf("FileList : ID = %s | value = %s", f->ID, f->value);

            if(f->info) {
                /* Debug des info */
                debugFileInfo(f->info, lvl+1);
            }

            f = f->next;

        }
    }

}

