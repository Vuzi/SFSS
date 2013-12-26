
#include "strTools.h"

/* =====================================================
                     removeStringPart

    m : message � d�couper
    start : d�but ou commencer
    lenght : longueur

    Coupe un message et renvoit le morceau, modifier
    �galement la chaine m.

   ===================================================== */
char *getStringPart(char* m, int start, int lenght) {

    char *part = NULL;

    /* Copie de la partie */
    part = malloc(sizeof(char)*(lenght+1));
    memcpy(part, m+start, (size_t)lenght);
    part[lenght] = '\0';

    return part;
}

/* =====================================================
                     getBracketContent

    m : message o� d�couper
    open : char de d�but
    close : char de fin

    D�coupe d'un message le contenu entre open et close
    en g�rant les inclusions (Avec open et close).
    Renvoit null en cas de non r�sultat. La chaine m
    est modifi�e.

   ===================================================== */

/* A corriger pour g�rer les echappements */
char* getBracketContent(char** m, char open, char close) {

    char *curs = *m, *tmp = *m;

    /* On est positionn� sur l'ouvrant */
    if(*m && **m == open ) {

        if(open != close) {
            /* Tant qu'on trouve un fermant */
            while((curs = strchr(curs+1, close))) {

                /* S'il n'y a plus d'ouvrants OU si ils sont apr�s le fermant */
                if(!(tmp = strchr(tmp+1, open)) || tmp > curs) {

                    tmp = getStringPart(*m, 1, (curs-(*m))-1);

                    *m = curs + 1;

                    return tmp;
                }
            }
        } else {
            /* Ne g�re pas les inclusions */
            if((curs = strchr(curs+1, close))) {

                    tmp = getStringPart(*m, 1, (curs-(*m))-1);

                    *m = curs + 1;

                    return tmp;
            }
        }
    }

    return NULL;
}


bool isBlank(char c) {
    return (c == ' ' || c == '\n' || c == '\t' || c == '\r');
}

char* firstNonBlank(char *m) {
    while(*m) {
        if(!isBlank(*m))
            return m;
        else
            m++;
    }
    return m;
}

char* getStringElem(char** m) {

    char *c = *m;
    size_t s = 0;

    while(*c && *c != ',' && *c != ':' && !isBlank(*c)) {
        c++;
    }

    s = c-*m;
    c = malloc(sizeof(char)*(s+1));
    strncpy(c, *m, s);
    c[s] = '\0';

    *m += s;

    return c;
}

/* =====================================================
                     getFileContent

    name : fichier � ouvrir

    Ouvre un fichier et renvoit son contenu dans une
    chaine de caract�re.

   ===================================================== */
char* getFileContent(char* name) {
    FILE *f = NULL;
    char *m = NULL;
    int l = 0;

    if((f = fopen(name, "rb"))){

        fseek(f, 0, SEEK_END);
        l = (int)ftell(f);
        m = malloc(sizeof(char)*(l+1));
        rewind(f);
        if(fread(m, 1, l, f) != (unsigned)l) {
            /* Erreur de lecture de fichier */
            HERROR_WRITE(HERROR_ERROR, "[Lecture fichier] Lecture du fichier '%s' impossible", name);
            fclose(f);
            free(m);
            return NULL;
        } else {
            fclose(f);
            m[l] = '\0';
            return m;
        }
    } else {
        /* Erreur d'ouverture de fichier */
        HERROR_WRITE(HERROR_ERROR, "[Lecture fichier] Ouverture du fichier '%s' impossible", name);
        return NULL;
    }
}

char* copy(char* str) {

    char* c = malloc(sizeof(char)*(strlen(str)+1));

    strcpy(c, str);

    return c;
}
