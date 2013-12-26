
#include "jsonToFileList.h"

FileList* JobjToFileList(Jobj *j) {

    FileList *f = NULL;

    Jdata *data = NULL;

    if(j) {
        /* On vérifie que la liste est de type folder_list */
        if((data = getNextJdataStringByName(j->data, "type")) && strcmp(data->d->s, "folder_list") == 0) {
            /* On recupere le Jdata "list" */
            if((data = getNextJdataJsonByName(j->data, "list")) && data->d->j) {
                HERROR_WRITE(HERROR_DEBUG, "[Json to FolderStruct] Lancement de la conversion");
                f = JdataToFileList(data->d->j->data);
                HERROR_WRITE(HERROR_DEBUG, "[Json to FolderStruct] Conversion terminee");
            } else {
                HERROR_WRITE(HERROR_ERROR, "[Json to FolderStruct] La liste n'est pas correctement formatee (Absence de l'element 'list'), impossible de la traduire");
            }
        } else {
            HERROR_WRITE(HERROR_ERROR, "[Json to FolderStruct] La liste n'est pas correctement formatee (Absence de 'type=folder_list'), impossible de la traduire");
        }
    } else {
        HERROR_WRITE(HERROR_WARNING, "[Json to FolderStruct] Liste vide (nulle), aucune conversion");
    }

    return f;
}

FileList *JdataToFileList(Jdata *data) {

    FileList *f = NULL;

    char* ID = NULL, *value = NULL, *name = NULL, *root = NULL;
    long long int size = 0, last_modif = 0;

    Jobj *j;
    Jdata *info = NULL;

    if(data) {
        data = data->first;

        while((data = getNextJdataJsonByName(data, "file")) && data->d->j) {

            /* Récupération du Jobj */
            j = data->d->j;

            /* Récupération et recopie du ID/value */
            ID = getAndCopyJdataString(j->data, "ID");
            value = getAndCopyJdataString(j->data, "value");

            if(ID && value) {

                f = addFileList(f, ID, value);

                if((info = getNextJdataJsonByName(j->data, "info"))) {
                    info = info->d->j->data;

                    /* Récupération et recopie du nom/root */
                    name = getAndCopyJdataString(info->first, "name");
                    root = getAndCopyJdataString(info->first, "root");

                    /* Taille */
                    info = getNextJdataIntByName(info->first, "size");

                    if(info && name && root) {
                        size = info->d->i;

                        /* last_modif */
                        info = getNextJdataIntByName(info->first, "last_modif");

                        if(info) {
                            last_modif = info->d->i;

                            f->info = newFileInfo(copy(name), copy(root), (unsigned)size, (int)last_modif);

                        } else {
                            HERROR_WRITE(HERROR_WARNING, "[Json to FolderStruct] Element avec Info mal formate ('%s')", ID);
                        }

                    } else {
                        HERROR_WRITE(HERROR_WARNING, "[Json to FolderStruct] Element avec Info mal formate ('%s')", ID);
                    }

                } else {
                    HERROR_WRITE(HERROR_WARNING, "[Json to FolderStruct] Element sans Info ('%s')", ID);
                }


            } else {
                HERROR_WRITE(HERROR_WARNING, "[Json to FolderStruct] Element sans ID et/ou value, impossible de le traiter");
            }

            data = data->next;
        }
    }

    return getFirstFileList(f);
}
