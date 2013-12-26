
#include "fileInfo.h"


/* =====================================================
                     newFileInfo

    Crée une nouvelle liste.

   ===================================================== */
FileInfo* newFileInfo(char *name, char* root, unsigned long long int size, int last_modif) {

    FileInfo *f = malloc(sizeof(FileInfo));

    f->name = name;
    f->root = root;
    f->size = size;
    f->last_modif = last_modif;

    return f;
}


/* =====================================================
                     deleteFileInfo

    f : liste à supprimer

    Supprime l'élément f

   ===================================================== */
void deleteFileInfo(FileInfo *f) {

    if(f) {

        /* Suppresion nom */
        free(f->name);

        /* Suppresion root */
        free(f->root);

        free(f);
    }

}


/* =====================================================
                        debugFileStruct

    f : liste à debuger
    lvl : nombre de tabulation

    Debug la liste f.

   ===================================================== */
void debugFileInfo(FileInfo *f, int lvl) {
    int i = 0;

    if(f) {

        putchar('\n');

        for(i = 0; i<lvl; i++)
            putchar('\t');

        printf("FileInfo : name = %s | root = %s | size = %lld | last_modif = %d", f->name, f->root, f->size, f->last_modif);
    }
}

