
#include "account.h"

int AccountIDtrack = -1;

Account* newAccount(char* login, char* pass) {

    return addAccount(NULL, login, pass);
}

Account* addAccount(Account *a, char* login, char* pass) {

    char* logFile = malloc(sizeof(char)*(12+strlen(login)));
    Account* newA = malloc(sizeof(Account));

    newA->login = login;
    newA->pass = pass;
    newA->ID = ++AccountIDtrack;

    newA->nb_user = 0;

    newA->options = NULL;
    newA->regst_ip = NULL;

    sprintf(logFile, "users/%s/log", login);

    if(!(newA->log = fopen(logFile, "a+")))
        HERROR_WRITE(HERROR_WARNING, "[Ouv. fichier log] Impossible d'ouvrir le fichier de log de l'utilisateur '%s'", login);

    free(logFile);

    if(a) {
        getLastAccount(a)->next = newA;
        newA->first = a->first;
    } else {
        newA->first = newA;
    }

    newA->next = NULL;

    return newA;
}

Account *removeAccount(Account *a, int ID) {

    Account *before = NULL, *after = NULL;

    if(a) {
        a = a->first;

        while(a && a->ID != ID) {
            before = a;
            a = a->next;
        }

        if(a) {
            after = a->next;

            deleteAccount(a);

            if(before)
                before->next = after;
            else if(after)
                after->first = after;

            if(!after && before)
                before->next = NULL;

            if(after)
                return after;
            else if(before)
                return before;
        }
    }
    return NULL;
}


void deleteAccount(Account *a) {
    if(a) {
        free(a->login);
        free(a->pass);
        free(a->options);
        deleteFileList(a->fileList);
        deleteAccount_ip(a->regst_ip);

        if(a->log)
            fclose(a->log);
    }
}

void loadAccountFileList(Account *a) {
    if(a) {
        Jobj* j = NULL;
        FILE* test = NULL;

        /* Nom du fichier */
        char *filename = malloc(sizeof(char)*(strlen(a->login)+15+2));
        strcpy(filename, "users/");
        strcat(filename, a->login);
        strcat(filename, "/file_list");

        if((test = fopen(filename, "rb"))) {
            /* Si la liste existe */
            fclose(test);
            HERROR_WRITE(HERROR_DEBUG, "[Charg. liste fichier] Chargement liste des fichiers ( login : '%s')", a->login);

            j = fileToJobj(filename);
            free(filename);

            a->fileList = JobjToFileList(j);
            a->act_size = getTotalSizeFileList(a->fileList);

            free(j);

            HERROR_WRITE(HERROR_DEBUG, "[Charg. liste fichier] Chargement liste des fichiers termine ( login : '%s')", a->login);
        } else {
            HERROR_WRITE(HERROR_WARNING, "[Charg. liste fichier] Aucune liste, creation d'une liste vierge ( login : '%s')", a->login);
            a->fileList = NULL;
            a->act_size = 0;
        }
    }
}

Account* getLastAccount(Account *a) {
    while(a && a->next) {
        a = a->next;
    }
    return a;
}

Jobj* accountToJobj(Account *a) {

    Jobj *j = NULL;
    Jdata *tmp = NULL;

    int i = 0;

    if(a) {
        j = newJobj();
        j->data = addJdataString(NULL, copy("type"), copy("account"));
        addJdataString(j->data, copy("login"), copy(a->login));
        addJdataString(j->data, copy("pass"), copy(a->pass));
        tmp = addJdataJson(j->data, copy("options"), newJobj());

            tmp->d->j->data = addJdataInt(NULL, copy("multi_user"), a->options->multi_user);
            addJdataInt(tmp->d->j->data, copy("max_size"), a->options->max_size);

        tmp = addJdataJson(j->data, copy("ip_list"), newJobj());

            for(i = 0; i < a->regst_ip->nb_ip; i++)
                tmp->d->j->data = addJdataString(tmp->d->j->data, copy("ip"), copy(a->regst_ip->ip[i]));

            if(tmp->d->j->data)
                tmp->d->j->data = tmp->d->j->data->first;
    }

    return j;

}

Account* JobjToAccount(Jobj *j) {

    Account *a = NULL;
    Jdata *data = NULL, *data_option = NULL;

    char *login = NULL, *pass = NULL;
    unsigned long long int multi_user = 0, max_size = 0;

    if(j) {
        /* On check la présence de type=account */
        if((data = getNextJdataStringByName(j->data, "type")) && strcmp("account", data->d->s) == 0 ) {
            if((data = getNextJdataStringByName(j->data, "login"))) {
                login = copy(data->d->s);
                if((data = getNextJdataStringByName(j->data, "pass"))) {
                    pass = copy(data->d->s);

                    /* Nouveau Account */
                    a = newAccount(copy(login), copy(pass));

                    /* Les options */
                    if((data = getNextJdataJsonByName(j->data, "options"))) {

                        if((data_option = getNextJdataIntByName(data->d->j->data, "multi_user")))
                           multi_user = data_option->d->i;

                        if((data_option = getNextJdataIntByName(data->d->j->data, "max_size")))
                           max_size = data_option->d->i;

                    }
                    a->options = newAccount_options(multi_user, max_size);

                    /* Les ip */
                    a->regst_ip = newAccount_ip();

                    if((data = getNextJdataJsonByName(j->data, "ip_list"))) {
                        data_option = data->d->j->data;

                        while((data_option = getNextJdataStringByName(data_option, "ip"))) {
                            addIpAccount_ip(a->regst_ip, copy(data_option->d->s));
                            data_option = data_option->next;
                        }
                    }

                    /* On renvoit */
                    return a;

                } else {
                    HERROR_WRITE(HERROR_ERROR, "[Charg. User] Chargement impossible : absence de 'pass' (Compte '%s')", login);
                    free(login);
                }
            } else {
                HERROR_WRITE(HERROR_ERROR, "[Charg. User] Chargement impossible : absence de 'login'");
            }
        } else {
            HERROR_WRITE(HERROR_ERROR, "[Charg. User] Chargement impossible : absence de 'type=account'");
        }
    }

    return a;
}

AccountConnectError connectUserToAccount(User *u, Account **a, char* login, char* pass) {

    Account *tmp = *a, *newA = NULL;
    AccountConnectError er = ACONNECT_NOERROR;

    int i = 0;

    /* On le cherche dans les comptes existant */
    while(tmp) {
        /* Test login */
        if(strcmp(login, tmp->login) == 0) {
            /* Test password */
            HERROR_WRITE(HERROR_DEBUG, "[Conn. User] Compte deja charge (compte '%s' | ip '%s')", login, u->ip);

            if(strcmp(pass, tmp->pass) == 0) {

                /* Tester si on autorise plusieurs connexions */
                if(tmp->options && tmp->options->multi_user != 0 && tmp->options->multi_user == tmp->nb_user)
                    return ACONNECT_LIMITUSER;

                /* Tester si l'ip est correcte */
                if(tmp->regst_ip && tmp->regst_ip->ip) {
                    for(i = 0; i < tmp->regst_ip->nb_ip ; i++) {
                        if(strcmp(tmp->regst_ip->ip[i], u->ip) == 0)
                            break;
                    }
                    if(i >= tmp->regst_ip->nb_ip)
                        return ACONNECT_BADIP;
                }

                /* Tout est bon */
                u->acc = tmp;
                u->acc->nb_user++;

                return ACONNECT_NOERROR;
            } else
                return ACONNECT_BADPASS;

        }
        tmp = tmp->next;
    }

    /* Sinon on doit le charger */

    HERROR_WRITE(HERROR_DEBUG, "[Conn. User] Chargement du compte (compte '%s' | ip '%s')", login, u->ip);
    er = getAndTestAccountFromFile(&newA, login, pass, u->ip);
    HERROR_WRITE(HERROR_DEBUG, "[Conn. User] Chargement compte termine (compte '%s' | ip '%s')", login, u->ip);

    if(er == ACONNECT_NOERROR) {

        /* Chargement valide */

        if(*a) {
            *a = getLastAccount(*a);
            (*a)->next = newA;
            newA->first = (*a)->first;
        } else
            *a = newA;

        u->acc = newA;
        u->acc->nb_user++;

        return ACONNECT_NOERROR;
    } else
        return er;
}

AccountConnectError getAndTestAccountFromFile(Account **a, char *login, char *pass, char* ip) {
    /* On commence par charger l'utilisateur en mémoire */

    Jobj *acc_tmp = NULL;
    FILE *test = NULL;

    int i = 0;

    char *filename = malloc(sizeof(char)*(strlen(login)+10+2));
    strcpy(filename, "users/");
    strcat(filename, login);
    strcat(filename, "/info");

    if((test = fopen(filename, "rb"))) {
        /* Si le compte existe */
        fclose(test);
        acc_tmp = fileToJobj(filename);
        free(filename);
        filename = NULL;

        (*a) = JobjToAccount(acc_tmp);
        deleteJobj(acc_tmp);

        if(strcmp((*a)->login, login) == 0) {
            if(strcmp((*a)->pass, pass) == 0) {

                if((*a)->regst_ip && (*a)->regst_ip->ip) {
                    for(i = 0; i < (*a)->regst_ip->nb_ip ; i++) {
                        if(strcmp((*a)->regst_ip->ip[i], ip) == 0)
                            break;
                    }
                    if(i >= (*a)->regst_ip->nb_ip) {
                        deleteAccount(*a);
                        (*a) = NULL;
                        return ACONNECT_BADIP;
                    }
                }
                /* Tout à été vérifié, connexion possible */
                return ACONNECT_NOERROR;

            } else {
                /* Mauvais password */
                deleteAccount(*a);
                (*a) = NULL;
                return ACONNECT_BADPASS;
            }
        } else {
            deleteAccount(*a);
            (*a) = NULL;
        }

    }

    if(filename)
        free(filename);

    return ACONNECT_BADLOGIN;
}

void removeUserAndAccount(User **u, Account **a) {

    if(*u) {
        if((*u)->acc) {
            if((*u)->acc->nb_user == 1)
                *a = removeAccount((*u)->acc, (*u)->acc->ID);
            else
                (*u)->acc->nb_user--;
        }

        *u = removeUser(*u);
    }
}


Account_options *newAccount_options(unsigned int multi_user, unsigned long long int max_size) {

    Account_options *u = malloc(sizeof(Account_options));

    u->multi_user = multi_user;
    u->max_size = max_size;

    return u;
}


Account_ip *newAccount_ip(void) {

    Account_ip *u = malloc(sizeof(Account_ip));

    u->ip = NULL;
    u->nb_ip = 0;

    return u;
}

void addIpAccount_ip(Account_ip *a, char *ip) {

    char **tmp = NULL;
    int i = 0;

    if(a && ip) {
        tmp = malloc(sizeof(char*)*(a->nb_ip+1));
        for(i = 0; i < a->nb_ip; i++)
            tmp[i] = a->ip[i];

        /* Nouvelle IP */
        tmp[i] = ip;

        if(a->ip)
            free(a->ip);

        /* Affectation */
        a->ip = tmp;
        a->nb_ip++;
    }

}

bool isInIpAccount_ip(Account_ip *a, char *ip) {
    int i = 0;

    if(a && ip) {

        for(i = 0; i < a->nb_ip; i++) {
            if(strcmp(ip, a->ip[i]) == 0)
                return true;
        }
    }
    return false;
}

void removeIpAccount_ip(Account_ip *a, char *ip) {

    char **tmp = NULL;
    int i = 0, j = 0;

    if(a && ip) {

        if(isInIpAccount_ip(a, ip)) {

            tmp = malloc(sizeof(char*)*(a->nb_ip-1));

            for(i = 0; i < a->nb_ip; i++) {
                if(strcmp(a->ip[i], ip) == 0) {
                    free(a->ip[i]);
                    j++;
                } else
                    tmp[i-j] = a->ip[i];
            }

            free(a->ip);
            a->ip = tmp;
            a->nb_ip -= j;
        }
    }

}

void deleteAccount_ip(Account_ip *a) {

    if(a) {
        int i = 0;

        for(i = 0; i < a->nb_ip; i++)
            free(a->ip[i]);

        free(a->ip);
        free(a);
    }
}

void logAccountInfo(User *u, char* title) {

    char buffer[256];
    time_t timestamp = time(NULL);

    if(u->acc && u->acc->log) {
        strftime(buffer, sizeof(buffer), "%d-%m-%y %H:%M:%S - ", localtime(&timestamp));
        fputc('\n', u->acc->log);
        fputs(buffer, u->acc->log);
        fputs(u->ip, u->acc->log);
        fputs(" [", u->acc->log);
        fputs(title, u->acc->log);
        fputs("] ", u->acc->log);
    }
}

void saveAccount(Account *a) {

    Jobj* j = NULL;

    char *filename = NULL;

    if(a) {
        HERROR_WRITE(HERROR_DEBUG, "[Sauv. compte] Lancement sauvegarde compte '%s'", a->login);

        j = accountToJobj(a);

        filename = malloc(sizeof(char)*(strlen(a->login)+10+2));
        strcpy(filename, "users/");
        strcat(filename, a->login);
        strcat(filename, "/info");

        JobjToFile(j, filename, 1);

        free(filename);
        deleteJobj(j);

        HERROR_WRITE(HERROR_DEBUG, "[Sauv. compte] Fin sauvegarde compte '%s'", a->login);
    }

}


void saveAccountFileList(Account *a) {

    Jobj* j = NULL;

    char *filename = NULL;

    if(a) {
        HERROR_WRITE(HERROR_DEBUG, "[Sauv. compte] Lancement sauvegarde liste fichier compte '%s'", a->login);

        j = fileListToJobj(a->fileList);

        filename = malloc(sizeof(char)*(strlen(a->login)+50));
        strcpy(filename, "users/");
        strcat(filename, a->login);
        strcat(filename, "/file_list");

        JobjToFile(j, filename, 0);

        free(filename);
        deleteJobj(j);

        HERROR_WRITE(HERROR_DEBUG, "[Sauv. compte] Fin sauvegarde liste fichier compte '%s'", a->login);
    }
}

bool checkTmpFile(User *u, FILE* F) {

    FileList *tmp = NULL;

    if(u->f_tmp && F) {
        rewind(F);
        /* A changer, pour les tests */
        if((unsigned)ftello64(F) == u->f_tmp->info->size || 1) {
            tmp = u->acc->fileList;

            while(tmp) {
                /* Erreur, fichier déjà présent */
                if(strcmp(tmp->ID, u->f_tmp->ID) == 0) {
                    /* S'il est locké c'est bon */
                    if(tmp->locked)
                        break;
                    else
                        return false;
                }

                tmp = tmp->next;
            }

            return true;
        }
    }

    return false;

}

FileList *getFileListFromMsg(Jobj *j) {

    FileList *f = NULL;

    if(j) {
        f = JdataToFileList(getNextJdataJsonByName(j->data, "file"));
    }

    return f;
}

AccountFileError checkFileToChange(FileList *f, User *u) {

    FileList *tmp = NULL;

    if(f && u) {
        tmp = u->acc->fileList;

        /* On doit trouver le fichier à modifier */
        while(tmp) {
            /* Le fichier est présent */
            if(strcmp(tmp->ID, f->ID) == 0)
                break;

            tmp = tmp->next;
        }

        /* Si le fichier existe */
        if(tmp) {
            /* Si le fichier n'est pas locké */
            if(!tmp->locked) {
                /* Si le poid est correct */
                if((f->info->size - tmp->info->size) + u->acc->act_size < u->acc->options->max_size || u->acc->options->max_size == 0) {
                    /* On lock le fichier */
                    tmp->locked = true;
                    /* Tout est bon */
                    return AADDFILE_NOERROR;
                } else
                    return AADDFILE_QUOTA;
            } else
                return AADDFILE_LOCKED;
        } else
            return AADDFILE_EXIST;
    }

    return AADDFILE_NOFILE;
}

AccountFileError deleteFileInFileList(FileList *f, User *u) {

    FileList *tmp = NULL;

    char* filename = NULL;

    if(f && u) {
        tmp = u->acc->fileList;

        /* On doit trouver le fichier à supprimer */
        while(tmp) {
            /* Fichier présent */
            if(strcmp(tmp->ID, f->ID) == 0)
                break;

            tmp = tmp->next;
        }

        /* Si le fichier existe */
        if(tmp) {
            /* Si le fichier n'est pas locké */
            if(!tmp->locked) {

                /* On supprime le fichier du disque */
                filename = malloc(sizeof(char)*(strlen(u->acc->login)+strlen(u->f_tmp->ID)+20));
                sprintf(filename, "users/%s/files/%s", u->acc->login, u->f_tmp->ID);

                /* On supprime */
                unlink(filename);
                free(filename);

                /* On supprime de la liste & on sauvegarde */
                u->acc->fileList = removeFileListElem(tmp);
                saveAccountFileList(u->acc);

                u->acc->act_size = getTotalSizeFileList(u->acc->fileList);

                return AADDFILE_NOERROR;

            } else
                return AADDFILE_LOCKED;
        } else
            return AADDFILE_EXIST;
    }

    return AADDFILE_NOFILE;
}


AccountFileError checkFileToAdd(FileList *f, User *u) {

    FileList *tmp = NULL;

    if(f && u) {
        if(f->info->size + u->acc->act_size < u->acc->options->max_size || u->acc->options->max_size == 0) {
            tmp = u->acc->fileList;

            while(tmp) {
                /* Erreur, fichier déjà présent */
                if(strcmp(tmp->ID, f->ID) == 0)
                    return AADDFILE_EXIST;

                tmp = tmp->next;
            }

            /* Tout est bon */
            return AADDFILE_NOERROR;

        } else {
            /* Erreur, au dessus du quota */
            return AADDFILE_QUOTA;
        }
    }

    return AADDFILE_NOFILE;
}


void debugAccount(Account *a) {

    int i = 0;

    if(a) {

        printf("\nAccount : login : %s | pass : %s\n\t", a->login, a->pass);

        if(a->options)
            printf("Options : multi_user : %d - max_size : %lld\n\t", a->options->multi_user, a->options->max_size);

        if(a->regst_ip) {
            for(i = 0; i < a->regst_ip->nb_ip; i++)
                printf("Ip : %s\n\t", a->regst_ip->ip[i]);
        }
    }
}
