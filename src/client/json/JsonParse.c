
#include "JsonParse.h"

static void JobjWriteIntoFileWithBlankChar(Jobj *j, FILE *f, int lvl);
static void JobjWriteIntoFile(Jobj *j, FILE *f, int lvl);

/* =====================================================
                    getNextJdataElemName

    m : liste


   ===================================================== */
char* getNextJdataElemName(char** m) {

    char* name = NULL;

    /*S'il s'agit d'un " alors on recupére le contenu */
    if(**m == '"') {

        /* On crée le nom */
        name = getBracketContent(m, '"', '"');

        /* On décale */
        *m = firstNonBlank(*m);
    }

    /* Sinon on découpe jusqu'à : */
    else {
        name = getStringElem(&(*m));
        *m = firstNonBlank(*m);
    }

    /* On renvoit le nom */
    return name;
}

/* =====================================================
                    getNextJdataElem

    list :
    s :


   ===================================================== */
Jdata* getNextJdataElem(Jdata *list, char **s) {

    long long int i = 0;
    float f = 0;
    char *c = NULL, *m = *s;

    if(m) {
        /* On se place sur le premier non-blank char */
        if(*(m = firstNonBlank(m))) {

            /* == Récupération du nom de l'élément == */
            list = addJdata(list, getNextJdataElemName(&m), NONE, NULL);

            /* == Récupération du contenu de l'élément == */
            if(*m == ':') {

                /* affectation */
                m++;
                m = firstNonBlank(m);

                /* Objet JSON */
                if(*m == '{') {
                    c = getBracketContent(&m, '{', '}');
                    addJdataData(list, JSON, (void*)parseToJobj(c));
                    free(c);
                }

                /* String */
                else if (*m == '"') {
                    c = getBracketContent(&m, '"', '"');
                    addJdataData(list, STRING, (void*)c);
                }

                /* String, int ou float */
                else {
                    c = getStringElem(&m);

                    switch(getType(c)) {

                        case NONE:
                            addJdataData(list, NONE, NULL);
                            break;

                        case NB_INT:
                            i = atoll(c);
                            addJdataData(list, NB_INT, (void*)(&i));
                            free(c);
                            break;

                        case NB_FLOAT:
                            f = (float)atof(c);
                            addJdataData(list, NB_FLOAT, (void *)(&f));
                            free(c);
                            break;

                        case STRING :
                            addJdataData(list, STRING, (void *)c);
                            break;

                        case JSON :

                        default :
                            HERROR_WRITE(HERROR_ERROR, "[Parsing JSON] Type inconnu/impossible");
                            addJdataData(list, NONE, NULL);
                            free(c);
                            break;
                    }
                }

                *s = m;
                return list;

            } else {
                /* Détruire liste */
                HERROR_WRITE(HERROR_ERROR, "[Parsing JSON] Declaration sans affectation");
                removeLastJdata(list);
                return NULL;
            }
        }
        else {
            return NULL;
        }
    }
    else {
        HERROR_WRITE(HERROR_ERROR, "[Parsing JSON] Chaine vide");
        return NULL;
    }
}

/* =====================================================
                      parseToJobj

    s :

   ===================================================== */
Jobj* parseToJobj(char *s) {

    Jobj *j = NULL;
    Jdata *tmp = NULL;

    if(s) {
        j = newJobj();

        while((tmp = getNextJdataElem(j->data, &s))) {
            s = firstNonBlank(s);

            if(*s == ',')
                s++;

            j->data = tmp;
        }

        if(j->data)
            j->data = j->data->first;
    }

    return j;
}


/* =====================================================
                        getType

    data :


   ===================================================== */
Jdata_type getType(char* data) {

    char p = 0;

    if(data) {
        if(strlen(data) == 0) {
            return NONE;
        } else {
            while(*data) {
                if (*data == '.') {
                    if(p>0)
                        return STRING; /* Si plus d'un point, string */
                    else
                        p++;
                }
                else if (*data < '0' || *data > '9')
                    return STRING; /* C'est un string */

                data++;
            }

            if(p == 1)
                return NB_FLOAT; /* Un point, alors float */
            else
                return NB_INT; /* Pas de point, entier */
        }
    } else
        return NONE;
}

Jobj* fileToJobj(char* filename) {

    char *s = NULL, *tmp = NULL, *bracket = NULL;
    Jobj *j = NULL;

    HERROR_WRITE(HERROR_DEBUG, "[File to Json] Lancement recuperation fichier '%s'", filename);

    if(filename) {
        if((s = getFileContent(filename))) {

            HERROR_WRITE(HERROR_DEBUG, "[File to Json] Fin recuperation");
            HERROR_WRITE(HERROR_DEBUG, "[File to Json] Lancement conversion en JSON");

            tmp = s;
            bracket = getBracketContent(&s, '{', '}');
            j = parseToJobj(bracket);

            free(tmp);
            free(bracket);

            HERROR_WRITE(HERROR_DEBUG, "[File to Json] Conversion terminee");

        } else
            HERROR_WRITE(HERROR_ERROR, "[File to Json] Impossible de recuperer le JSON du fichier '%s'", filename);

    } else
        HERROR_WRITE(HERROR_ERROR, "[File to Json] Impossible de recuperer le JSON du fichier '%s'", filename);

    return j;
}

void JobjToFile(Jobj *j, char* filename, char mode) {
    FILE *f = NULL;

    if(j) {
        if((f = fopen(filename, "wb")) != NULL) {
            HERROR_WRITE(HERROR_DEBUG, "[JSON to file] Lancement ecriture JSON dans fichier '%s'", filename);

            if(mode == 0)
                JobjWriteIntoFile(j, f, 0);
            else if (mode == 1)
                JobjWriteIntoFileWithBlankChar(j, f, 0);
            else {
                JobjWriteIntoFile(j, f, 0);
                HERROR_WRITE(HERROR_WARNING, "[JSON to file] Impossible d'utiliser le mode selectionne, utilisation du mode '0'");
            }

            fclose(f);
            HERROR_WRITE(HERROR_DEBUG, "[JSON to file] Fin de l'ecriture");
        } else {
            HERROR_WRITE(HERROR_ERROR, "[JSON to file] Impossible d'ouvrir le fichier '%s'", filename);
        }
    } else {
        HERROR_WRITE(HERROR_WARNING, "[JSON to file] Liste vide a ecrire dans le fichier '%s', aucune operation effectuee", filename);
    }
}


static void JobjWriteIntoFileWithBlankChar(Jobj *j, FILE *f, int lvl) {

    int i = 0, k = 0;
    Jdata* data = NULL;

    char buffer[256];

    fputs("{\n", f);

    data = j->data;

    while(data) {

        if(k > 0)
            fputs(",\n", f);

        k++;

        for(i = 0; i < lvl+1; i++)
            fputs("\t", f);

        fputs("\"", f);
        fputs(data->name, f);
        fputs("\":", f);

        switch(data->type) {

            case STRING :

                fputs("\"", f);
                fputs(data->d->s, f);
                fputs("\"", f);

                break;

            case NB_FLOAT :

                sprintf(buffer, "%f", data->d->f);
                fputs(buffer, f);

                break;

            case NB_INT :

                sprintf(buffer, "%lld", data->d->i);
                fputs(buffer, f);

                break;

            case JSON :
                JobjWriteIntoFileWithBlankChar(data->d->j, f, lvl+1);
                break;

            case NONE :
                break;

            default :
                break;
        }


        data = data->next;
    }

    fputs("\n", f);

    for(i = 0; i < lvl; i++)
        fputs("\t", f);

    fputs("}", f);
}


static void JobjWriteIntoFile(Jobj *j, FILE *f, int lvl) {

    int k = 0;
    Jdata* data = NULL;

    char buffer[256];

    fputs("{", f);

    data = j->data;

    while(data) {

        if(k > 0)
            fputs(",", f);

        k++;

        fputs("\"", f);
        fputs(data->name, f);
        fputs("\":", f);

        switch(data->type) {

            case STRING :

                fputs("\"", f);
                if(data->d->s)
                    fputs(data->d->s, f);
                fputs("\"", f);

                break;

            case NB_FLOAT :

                sprintf(buffer, "%f", data->d->f);
                fputs(buffer, f);

                break;

            case NB_INT :

                sprintf(buffer, "%lld", data->d->i);
                fputs(buffer, f);

                break;

            case JSON :
                JobjWriteIntoFile(data->d->j, f, lvl+1);
                break;

            case NONE :
                break;

            default :
                break;
        }


        data = data->next;
    }

    fputs("}", f);
}

char* JobjToChar(Jobj *j) {

    FILE *tmp = NULL;

    if(j) {

        if((tmp = fopen("~tmp", "wb"))) {
            JobjWriteIntoFile(j, tmp, 0);
            fclose(tmp);
            return getFileContent("~tmp");
        } else {
            HERROR_WRITE(HERROR_ERROR, "[JSON to Str] Impossible d'ouvrir le fichier temporaire '~tmp', aucune operation effectuee");
        }
    } else {
        HERROR_WRITE(HERROR_WARNING, "[JSON to Str] Liste vide a convertir, aucune operation effectuee");
    }
    return NULL;
}
