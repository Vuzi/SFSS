
#include "dirToFileList.h"

FileList* startFileListing(char* dir) {

    FileList *f = NULL;

    if(dir) {
        HERROR_WRITE(HERROR_DEBUG, "[File. listing] Debut du listing du dossier '%s'", dir);
        f = getFileList(strlen(dir), dir, NULL);
        HERROR_WRITE(HERROR_DEBUG, "[File. listing] Fin du listing du dossier '%s'", dir);
    } else {
        HERROR_WRITE(HERROR_ERROR, "[File. listing] Erreur, le nom du dossier a tester est null");
    }

    if(f)
        return f->first;
    else
        return f;
}

FileList* getFileList(int rootLenght, char *dir, FileList *f) {

    /* Dossier */
    DIR* rep = opendir(dir);
    struct dirent* file = NULL;

    /* Info sur les fichiers */
    struct __stat64 s;
    char* name;

    /* Si on ouvre le répertoire */
    if(rep) {
        while ((file = readdir(rep)) != NULL) {
            /* On vire . & .. et tout les trucs cachés */
            if(file->d_name[0] != '.' && file->d_name[0] != '~') {

                /* Construction du nom du fichier/dossier */
                name = malloc(sizeof(char)*(strlen(dir)+strlen(file->d_name)+2));
                *name = '\0';
                strcat(name, dir);
                strcat(name, "/");
                strcat(name, file->d_name);

                if(_stat64(name, &s) == -1) {
                    HERROR_WRITE(HERROR_ERROR, "[File. listing] Erreur lors de la recuperation des stats du fichier '%s'", name);
                } else {
                    if(S_ISREG(s.st_mode)) {
                        /* Fichier */

                        /* Construction des informations du fichier */
                        f = makeFileListFromStat(f, name, dir+rootLenght, file->d_name, &s);

                    } else if (S_ISDIR(s.st_mode)) {
                        /* Dossier */
                        f = getFileList(rootLenght, name, f);
                    }
                }

                free(name);
            }
        }

        closedir(rep);

    } else {
        HERROR_WRITE(HERROR_ERROR, "[File. listing] Impossible d'ouvrir le repertoire '%s'", dir);
    }

    return getFirstFileList(f);

}

FileList *makeFileListFromStat(FileList *f, char* name, char* dir, char* filename, struct __stat64* s) {

	md5_state_t state;
	md5_byte_t digest[16];
	char *ID = malloc(sizeof(char)*(16*2 + 1));
	char *value = malloc(sizeof(char)*(16*2 + 1));
	char buffer[256];

	int i;

    /* Génération du MD5 du nom */
	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)name, strlen(name));
	md5_finish(&state, digest);

	for (i = 0; i < 16; ++i)
	    sprintf(ID + i * 2, "%02x", digest[i]);

    /* Génération du MD5 de value */
	md5_init(&state);

	md5_append(&state, (const md5_byte_t *)filename, strlen(filename)); /* filename */

	sprintf(buffer, "%lld", (long long int)s->st_size);
	md5_append(&state, (const md5_byte_t *)buffer, strlen(buffer)); /* poid */

	sprintf(buffer, "%lld", (long long int)s->st_mtime);
	md5_append(&state, (const md5_byte_t *)buffer, strlen(buffer)); /* date de modif */

	md5_finish(&state, digest);

	for (i = 0; i < 16; ++i)
	    sprintf(value + i * 2, "%02x", digest[i]);

    /* On ajoute le fichier */
    f = addFileList(f, ID, value);

    /* Les info du fichier */
    f->info = newFileInfo(copy(filename), copy(dir), s->st_size, s->st_mtime);

    return f;
}
