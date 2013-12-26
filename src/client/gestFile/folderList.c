
#include "folderList.h"

Jobj* makeDirList(char* root) {

    /* Nouvelle liste */
    Jobj* list = newJobj();

    /* Repertoire */

    struct __stat64 s;

    if(_stat64(root, &s) == -1) {
        /* Impossible d'ouvrir le dossier racine */
        HERROR_WRITE(HERROR_ERROR, "[File. listing] Impossible d'ouvrir le dossier racine \"%s\"", root);
        return NULL;
    } else {

        /* Info du dossier racine */
        list->data = addJdataString(NULL, "type", "folder_list");
        addJdataString(list->data, "root", root);
        addJdataInt(list->data, "time", (unsigned long long int)time(NULL));

        /* Construction liste dossiers/fichiers */
        makeDirContentList(addJdataJson(list->data, "folder", makeNewJobjDir(root, "", &s))->d->j, root);

        list->data = list->data->first;

        return list;
    }
}

void makeDirContentList(Jobj* container, char* dir) {

    /* Repertoire */
    DIR* rep = opendir(dir);
    struct dirent* file = NULL;
    struct __stat64 *s = NULL;
    char* new_dir = NULL, *name = NULL;

    /* Si on a un container */
    if(container != NULL) {

        /* Si on a réussi à ouvrir le dossier */
        if (rep == NULL) {
            HERROR_WRITE(HERROR_ERROR, "[File. listing] Impossible d'ouvrir le dossier \"%s\"", dir);
            return;
        } else {

            //puts("Dossier ouvert");

            while ((file = readdir(rep)) != NULL) {

                if(file->d_name[0] != '.') {
                    if((s = getFileStat(dir, file->d_name))) {
                        if(S_ISREG(s->st_mode)) {
                            /* Fichier */

                            /* Sauvegarde nom */
                            name = malloc(sizeof(char)*(strlen(file->d_name)+1));
                            strcpy(name, file->d_name);

                            /* Construction JSON */
                            container->data = addJdataJson(container->data, "file", makeNewJobjFile(dir, name, s));
                            container->data = container->data->first;

                        }
                        else if (S_ISDIR(s->st_mode)) {
                            /* Dossier */

                            /* Nouvelle racine (à conserver) */
                            new_dir = malloc(sizeof(char)*(strlen(dir)+strlen(file->d_name)+2));
                            strcpy(strcpy(strcpy(new_dir, dir)+strlen(new_dir), "/")+1, file->d_name);

                            /* Sauvegarde nom */
                            name = malloc(sizeof(char)*(strlen(file->d_name)+1));
                            strcpy(name, file->d_name);

                            /* Relancement procédure pour ce dossier & Construction JSON */
                            makeDirContentList(addJdataJson(container->data, "folder", makeNewJobjDir(dir, name, s))->d->j, new_dir);

                            container->data = container->data->first;
                        }

                        free(s);

                    } else {
                        HERROR_WRITE(HERROR_ERROR, "[File. listing] Erreur lors de la lecture du dossier \"%s\"", dir);
                    }
                }
            }


            if (closedir(rep) == -1)
                HERROR_WRITE(HERROR_ERROR, "[File. listing] Erreur lors de la fermeture du dossier \"%s\"", dir);

            return;
        }
    }
}


Jobj* makeNewJobjDir(char* root, char* name, struct __stat64* s) {

    Jobj* j = newJobj();

    j->data = addJdataString(NULL, "type", "folder");
    addJdataString(j->data, "name", name);
    addJdataString(j->data, "root", root);
    addJdataInt(j->data, "size", (unsigned long long int)s->st_size);
    addJdataInt(j->data, "last_access", (unsigned long long int)s->st_atime);
    addJdataInt(j->data, "last_modif", (unsigned long long int)s->st_mtime);
    addJdataInt(j->data, "last_stat_change", (unsigned long long int)s->st_ctime);

    return j;
}



Jobj* makeNewJobjFile(char* root, char* name, struct __stat64* s) {

    Jobj* j = newJobj();

    j->data = addJdataString(NULL, "type", "file");
    addJdataString(j->data, "name", name);
    addJdataString(j->data, "root", root);
    addJdataInt(j->data, "size", (unsigned long long int)s->st_size);
    addJdataInt(j->data, "last_access", (unsigned long long int)s->st_atime);
    addJdataInt(j->data, "last_modif", (unsigned long long int)s->st_mtime);
    addJdataInt(j->data, "last_stat_change", (unsigned long long int)s->st_ctime);

    return j;
}


struct __stat64* getFileStat(char* dir,  char* filename) {

    char* name = NULL;
    struct __stat64* s = NULL;

    if(dir && filename) {
        /* Construction du nom du fichier/dossier */
        name = malloc(sizeof(char)*(strlen(dir)+strlen(filename)+2));
        strcpy(strcpy(strcpy(name, dir)+strlen(name), "/")+1, filename);

        /* Construction stat */
        s = malloc(sizeof(struct __stat64));

        if(_stat64(name, s) == -1) {
            HERROR_WRITE(HERROR_ERROR, "[File. listing] Erreur lors de la recuperation des informations du fichier \"%s\" du dossier \"%s\"", filename, dir);
            free(name);
            return NULL;
        } else {
            free(name);
            return s;
        }
    } else {
        return NULL;
    }

}
