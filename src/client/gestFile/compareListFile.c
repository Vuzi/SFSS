
#include "compareListFile.h"

FileListComp *compareFileList(FileList *newList, FileList *oldList) {

    FileListComp *f = NULL;
    FileList *tmp = NULL, *newListTmp = newList;

    HERROR_WRITE(HERROR_DEBUG, "[Comp. FileList] Lancement de la comparaison");

    f = newFileListComp();

    if(newListTmp)
        newList = newList->first;

    if(oldList)
        oldList = oldList->first;

    while(newList) {
        if((tmp = getFileListByID(oldList, newList->ID))) {
            if(!strcmp(newList->ID, tmp->ID)) {
                /* Si value différent = modif */
                if(strcmp(newList->value, tmp->value))
                    pushFileListCompModified(f, newList);
            }
        } else {
            /* Pas dans l'ancienne liste = ajouté */
            pushFileListCompAdded(f, newList);
        }
        newList = newList->next;
    }

    if(newListTmp)
        newList = newListTmp->first;

    while(oldList) {
        if(!isInFileList(newList, oldList->ID)) {
            /* Pas dans la nouvelle liste = supprimé */
            pushFileListCompDeleted(f, oldList);
        }
        oldList = oldList->next;
    }

    HERROR_WRITE(HERROR_DEBUG, "[Comp. FileList] Comparaison terminee");


    return f;
}

/*
FileListComp *compareFileListOld(FileList *newList, FileList *oldList) {

    FileListComp *f = NULL;


    HERROR_WRITE(HERROR_DEBUG, "[Comp. FileList] Lancement de la comparaison");

    f = newFileListComp();

    if(newList)
        newList = newList->first;

    if(oldList)
        oldList = oldList->first;

    while(newList && oldList) {

        if(!strcmp(newList->ID, oldList->ID)) {
            if(strcmp(newList->value, oldList->value))
                pushFileListCompModified(f, newList);

            newList = newList->next;
            oldList = oldList->next;
        } else {
            if(!isInFileList(oldList, newList->ID)) {
                pushFileListCompAdded(f, newList);
                newList = newList->next;
            } else
                newList = newList->next;


            if(!isInFileList(newList, oldList->ID)) {
                pushFileListCompDeleted(f, oldList);
                oldList = oldList->next;
            }
        }
    }

    if(newList) {
        while(newList) {
            pushFileListCompAdded(f, newList);
            newList = newList->next;
        }
    } else if (oldList) {
        while(oldList) {
            pushFileListCompDeleted(f, oldList);
            oldList = oldList->next;
        }
    }

    HERROR_WRITE(HERROR_DEBUG, "[Comp. FileList] Comparaison terminee");


    return f;
}
*/
