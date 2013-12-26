
#include "caccount.h"

void deleteClientAccount(C_account *c) {

    if(c) {
        if(c->addr)
            free(c->addr);

        if(c->login)
            free(c->login);

        if(c->pass)
            free(c->pass);

        free(c);
    }
}

C_account *loadClientAccount(void) {

    char* filename = "account_info";
    FILE *f = fopen(filename, "r");

    Jobj *j = NULL;
    Jdata *data = NULL;

    C_account *c = NULL;

    /* On vérifie la présence du fichier */
    if(f) {
        HERROR_WRITE(HERROR_DEBUG, "[Chargement compte] Lancement chargement compte");
        fclose(f);
        j = fileToJobj(filename);

        if(j) {

            if((data = getNextJdataStringByName(j->data, "type"))) {
                c = calloc(1,sizeof(C_account));

                if((data = getNextJdataStringByName(j->data, "login"))) {
                    c->login = copy(data->d->s);

                    if((data = getNextJdataStringByName(j->data, "pass"))) {
                        c->pass = copy(data->d->s);

                        if((data = getNextJdataStringByName(j->data, "serv_addr"))) {
                            c->addr = copy(data->d->s);

                            if((data = getNextJdataIntByName(j->data, "serv_port"))) {
                                c->port = data->d->i;

                                /* Tout est là on peut renvoyer */
                                return c;
                            }

                            free(c->addr);
                        }
                        free(c->pass);
                    }

                    free(c->login);
                }
                free(c);
            }

            HERROR_WRITE(HERROR_FATAL, "[Chargement compte] Erreur, impossible de traiter le contenu du fichier d'information du compte (Des informations sont manquantes)");


        } else
            HERROR_WRITE(HERROR_FATAL, "[Chargement compte] Erreur, impossible de traiter le contenu du fichier d'information du compte");

    } else
        HERROR_WRITE(HERROR_FATAL, "[Chargement compte] Erreur, impossible d'ouvrir le fichier d'information du compte");

    return NULL;

}
