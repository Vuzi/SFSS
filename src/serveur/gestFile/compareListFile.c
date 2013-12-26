
#include "compareListFile.h"

FileListComp *compareFileList(FileList *newList, FileList *oldList) {

    FileListComp *f = NULL;


    HERROR_WRITE(HERROR_DEBUG, "[Comp. FileList] Lancement de la comparaison");

    f = newFileListComp();

    if(newList)
        newList = newList->first;

    if(oldList)
        oldList = oldList->first;

    while(newList && oldList) {
        /* S'il s'agit du même fichier */
        if(!strcmp(newList->ID, oldList->ID)) {
            if(strcmp(newList->value, oldList->value))
                pushFileListCompModified(f, newList);

            newList = newList->next;
            oldList = oldList->next;
        } else {
            if(!isInFileList(oldList->next, newList->ID)) {
                pushFileListCompAdded(f, newList);
                newList = newList->next;
            }

            if(!isInFileList(newList->next, oldList->ID)) {
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
            pushFileListCompAdded(f, oldList);
            oldList = oldList->next;
        }
    }

    HERROR_WRITE(HERROR_DEBUG, "[Comp. FileList] Comparaison terminee");


    return f;
}
