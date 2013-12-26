
#include "fileListComp.h"


/* =====================================================
                      newFileListComp

   ===================================================== */
FileListComp* newFileListComp(void) {

    FileListComp *f = malloc(sizeof(FileListComp));

    f->addedFiles = NULL;
    f->nbAddedFiles = 0;

    f->modifiedFiles = NULL;
    f->nbModifiedFiles = 0;

    f->deletedFiles = NULL;
    f->nbDeletedFiles = 0;

    return f;
}

void pushFileListCompAdded(FileListComp *f, FileList *l) {

    int i= 0;

    FileList **tmp = NULL;

    if(l) {
        if(f->addedFiles) {
            /* Tableau plus grand */
            tmp = malloc(sizeof(FileList*)*(f->nbAddedFiles + 1));

            /* On recopie */
            for(i = 0; i < f->nbAddedFiles; i++) {
                tmp[i] = f->addedFiles[i];
            }

            /* On ajoute */
            tmp[i] = l;

            /* On change */
            free(f->addedFiles);
            f->addedFiles = tmp;

            f->nbAddedFiles++;

        } else {

            /* Premier element */
            f->addedFiles = malloc(sizeof(FileList*));
            f->addedFiles[0] = l;
            f->nbAddedFiles++;

        }
    } else {
        HERROR_WRITE(HERROR_WARNING, "[Comp. FileList] Demande d'ajout d'element null, impossible de l'ajouter");
    }
}

void pushFileListCompModified(FileListComp *f, FileList *l) {

    int i= 0;

    FileList **tmp = NULL;

    if(l) {
        if(f->modifiedFiles) {
            /* Tableau plus grand */
            tmp = malloc(sizeof(FileList*)*(f->nbModifiedFiles + 1));

            /* On recopie */
            for(i = 0; i < f->nbModifiedFiles; i++) {
                tmp[i] = f->modifiedFiles[i];
            }

            /* On ajoute */
            tmp[i] = l;

            /* On change */
            free(f->modifiedFiles);
            f->modifiedFiles = tmp;

            f->nbModifiedFiles++;

        } else {

            /* Premier element */
            f->modifiedFiles = malloc(sizeof(FileList*));
            f->modifiedFiles[0] = l;
            f->nbModifiedFiles++;

        }
    } else {
        HERROR_WRITE(HERROR_WARNING, "[Comp. FileList] Demande d'ajout d'element null, impossible de l'ajouter");
    }
}

void pushFileListCompDeleted(FileListComp *f, FileList *l) {

    int i= 0;

    FileList **tmp = NULL;

    if(l) {
        if(f->deletedFiles) {
            /* Tableau plus grand */
            tmp = malloc(sizeof(FileList*)*(f->nbDeletedFiles + 1));

            /* On recopie */
            for(i = 0; i < f->nbDeletedFiles; i++) {
                tmp[i] = f->deletedFiles[i];
            }

            /* On ajoute */
            tmp[i] = l;

            /* On change */
            free(f->deletedFiles);
            f->deletedFiles = tmp;

            f->nbDeletedFiles++;

        } else {

            /* Premier element */
            f->deletedFiles = malloc(sizeof(FileList*));
            f->deletedFiles[0] = l;
            f->nbDeletedFiles++;

        }
    } else {
        HERROR_WRITE(HERROR_WARNING, "[Comp. FileList] Demande d'ajout d'element null, impossible de l'ajouter");
    }
}

/* =====================================================
                     deleteFileListComp

    f : à supprimer


   ===================================================== */
void deleteFileListComp(FileListComp *f) {

    if(f) {

        if(f->addedFiles)
            free(f->addedFiles);

        if(f->modifiedFiles)
            free(f->modifiedFiles);

        if(f->deletedFiles)
            free(f->deletedFiles);
    }
}


/* =====================================================
                        debugFileListComp

    f : liste à debuger
    lvl : nombre de tabulation

    Debug la liste f.

   ===================================================== */
void debugFileListComp(FileListComp *f) {
    int i = 0;

    if(f) {

        printf("-debugFileList :\n\tFilesAdded :\n");

        for( i=0 ; i < f->nbAddedFiles ; i++) {
            if(f->addedFiles[i]->info)
                printf("\t name : %s | root : %s \n", f->addedFiles[i]->info->name, f->addedFiles[i]->info->root);
        }

        printf("\tFilesModified :\n");

        for( i=0 ; i < f->nbModifiedFiles ; i++) {
            if(f->modifiedFiles[i]->info)
                printf("\t name : %s | root : %s \n", f->modifiedFiles[i]->info->name, f->modifiedFiles[i]->info->root);
        }

        printf("\tFilesDeleted :\n");

        for( i=0 ; i < f->nbDeletedFiles ; i++) {
            if(f->deletedFiles[i]->info)
                printf("\t name : %s | root : %s \n", f->deletedFiles[i]->info->name, f->deletedFiles[i]->info->root);
        }
    }
}

