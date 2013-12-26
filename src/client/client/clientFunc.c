
#include "clientFunc.h"


void sock_error(const char* str) {

    #ifdef __WIN32
        HERROR_WRITE(HERROR_ERROR, "[Erreur socket] %s : '%d' de WSA", str, WSAGetLastError());
        HERROR_STOP;
        exit(WSAGetLastError());
    #elif defined (__unix__)
        HERROR_WRITE(HERROR_ERROR, "[Erreur socket] %s : %s", str, strerror(errno));
        HERROR_STOP;
        exit(errno);
    #endif

}

bool makeNewEmptyFileListFile(void) {
    FILE *f = NULL;

    if((f = fopen("filelist", "w"))) {
        fputs("{\"type\":\"folder_list\",\"list\":{}}", f);
        fclose(f);
        return true;
    } else
        return false;
}

bool getFileFromServer(C_account *usr, FileList* toRcv) {

    char *filename = NULL, *final_filename = NULL, buffer[BUFFER_SIZE] = {0};
    struct utimbuf newTime;
    int n = 0;

    FILE* f = NULL;

    if(usr && toRcv) {

        HERROR_WRITE(HERROR_DEBUG, "[Charg. Fichier] Debut reception fichier");

        filename = malloc(sizeof(char)*(20+strlen(toRcv->ID)));
        sprintf(filename, "tmp/%s_%d", toRcv->ID, (int)usr->sock);

        final_filename = malloc(sizeof(char)*(strlen(usr->checkFolder)+1+strlen(toRcv->info->root)+1+strlen(toRcv->info->name)));
        if(strlen(toRcv->info->root) > 0)
            sprintf(final_filename, "%s/%s/%s", usr->checkFolder, toRcv->info->root, toRcv->info->name);
        else
            sprintf(final_filename, "%s/%s", usr->checkFolder, toRcv->info->name);

        if((f = fopen(filename, "wb"))) {

            while(1) {

                if((n = recv(usr->sock, buffer, BUFFER_SIZE, 0)) < 0)
                    sock_error("[Charg. Fichier] Erreur lors de la réception, impossible de continuer");

                fwrite(buffer, n, 1, f);

                /* Si on arrive en fin de fichier */
                if(ftello64(f) >= (off64_t)toRcv->info->size) {
                    HERROR_WRITE(HERROR_INFO, "[Charg. Fichier] Fin reception fichier");
                    break;
                }
            }

            fclose(f);

            /* On supprime l'ancien fichier s'il existe */
            unlink(final_filename);

            if(rename(filename, final_filename) == 0) {

                /* On change la date de modification du fichier pour correspondre */
                newTime.actime = time(NULL);
                newTime.modtime = (time_t)toRcv->info->last_modif;

                if (utime(final_filename, &newTime) != 0) {
                    HERROR_WRITE(HERROR_WARNING, "[Charg. Fichier] Impossible de modifier la date du fichier '%s' : ", final_filename);
                }

                free(filename);
                free(final_filename);

                return true;
            } else {
                HERROR_WRITE(HERROR_ERROR, "[Charg. Fichier] Impossible de renommer le fichier temporaire en '%s'", final_filename);
            }

        } else {
            HERROR_WRITE(HERROR_ERROR, "[Charg. Fichier] Impossible d'ouvrir le fichier temporaire '%s'", filename);

            /* On doit quand même recevoir le fichier */
            while(1) {

                if((n = recv(usr->sock, buffer, BUFFER_SIZE, 0)) < 0)
                    sock_error("[Charg. Fichier] Erreur lors de la réception, impossible de continuer")

                /* Si on arrive en fin de fichier */
                if(ftello64(f) >= (off64_t)toRcv->info->size) {
                    HERROR_WRITE(HERROR_INFO, "[Charg. Fichier] Fin reception fichier");
                    break;
                }
            }
        }

        /* On supp. le fichier temp */
        unlink(filename);

        free(filename);
        free(final_filename);
    }

    return false;

}

void sendFileToServer(SOCKET srvSock, char* folder, FileList* toSend) {

    FILE *f = NULL;
    char* name = calloc(sizeof(char)*(strlen(folder)+strlen(toSend->info->root)+strlen(toSend->info->name)+3), 1);
    char buffer[BUFFER_SIZE];
    int n = 0;

    printf("\n>Debut envoit fichier");
    getchar();

    /* Construction Nom */
    strcat(name, folder);

    if(strlen(toSend->info->root) > 0) {
        strcat(name, "/");
        strcat(name, toSend->info->root);
    }

    strcat(name, "/");
    strcat(name, toSend->info->name);

    /* Ouverture fichier */
    if((f = fopen(name, "rb"))) {

        /* Envoit fichier */
        while((n = fread(buffer, sizeof(char), BUFFER_SIZE, f)) == BUFFER_SIZE) {
           if(send(srvSock, buffer, BUFFER_SIZE, 0) < 0)
              sock_error("send()");
        }

        if(send(srvSock, buffer, n, 0) < 0)
            sock_error("send()");

        fclose(f);

        HERROR_WRITE(HERROR_DEBUG, "[Envoit Fic.] Envoit fichier '%s' effectue", name);

    } else {
        HERROR_WRITE(HERROR_ERROR, "[Envoit Fic.] Impossible d'ouvrir le fichier '%s', envoit d'un fichier vide a sa place", name);

        buffer[0] = EOF;

        if(send(srvSock, buffer, 1, 0) < 0)
              sock_error("send()");
    }
}

static void sendStringToServer(SOCKET srvSock, char *buffer) {

    /* Ici découper l'envoi en 4096 par 4096 A FAIRE §§*/

   if(send(srvSock, buffer, strlen(buffer), 0) < 0)
      sock_error("send()");
}


void sendMsgToServer(SOCKET srvSock, Msg *m) {

    Jobj *j = NULL;
    char *c = NULL;

    j = MsgToJobj(m);
    c = JobjToChar(j);

    deleteJobj(j);
    deleteMsg(m);

    sendStringToServer(srvSock, c);

    free(c);
}

// Fonction qui check une validation de message A FAIRE
bool getSuccessResponse(SOCKET srvSock, Msg_type typeResponse, char** responseString) {

    bool ret;

    Msg *m = NULL;
    Jdata *data = NULL;

    /* Si on a une réponse du serveur */
    if((m = getMsgResponse(srvSock))) {

        /* Du bon type */
        if(m->type == MSG_OPSUCCESS || m->type == MSG_OPFAIL) {

            /* Check du type type */
            if(m->data && (data = getNextJdataStringByName(m->data->data, "type"))) {
                if(!(getMsg_Type(data->d->s) == typeResponse)) {
                    HERROR_WRITE(HERROR_WARNING, "[Rcp. Message] Mauvais type de reponse du serveur");
                }
            } else {
                HERROR_WRITE(HERROR_WARNING, "[Rcp. Message] Pas de type de reponse");
            }

            /* Si on a un message */
            if(m->data && (data = getNextJdataStringByName(m->data->data, "msg"))) {
                *responseString = copy(data->d->s);
            } else {
                *responseString = copy("[Message vide]");
            }

            /* Réponse favorable */
            if(m->type == MSG_OPSUCCESS) {
                HERROR_WRITE(HERROR_DEBUG, "[Rcp. Message] Message favorable du serveur : '%s'", *responseString);
                ret = true;
            } else {
                HERROR_WRITE(HERROR_DEBUG, "[Rcp. Message] Message defavorable du serveur : '%s'", *responseString);
                ret = false;
            }
        } else {
            /* Message de type innatendu, on ne traite pas */
            *responseString = copy("Erreur lors de la recuperation du message, type innatendu");
            HERROR_WRITE(HERROR_WARNING, "[Rcp. Message] Message de type innatendu (%d), impossible de le traiter", m->type);
            ret = false;
        }

        deleteMsg(m);

    } else {
        *responseString = copy("Erreur lors de la recuperation du message");
        ret = false;
    }

    return ret;
}

Msg *getMsgResponse(SOCKET srvSock) {

    Jobj *j = NULL;
    Msg *m = NULL;
    char *buffer = NULL, *tmp = NULL, *bracket = NULL;

    Jdata *data = NULL;

    if(getServerResponse(srvSock, &buffer)) {

        tmp = buffer;
        bracket = getBracketContent(&buffer, '{', '}');
        j = parseToJobj(bracket);

        if(bracket)
            free(bracket);

        if(tmp)
            free(tmp);

        if(j) {
            m = JobjToMsg(j);

            /* On 'coupe' la liste */
            if((data = getNextJdataByName(j->data, "data")))
                data->type = NONE;

            deleteJobj(j);

            if(m)
                return m;
        }
    }

    HERROR_WRITE(HERROR_ERROR, "[Rcp. Message] Impossible de lire le message");
    return NULL;

}

bool getServerResponse(SOCKET srvSock, char **buffer) {

    int t = 0, n = 0, i = 0;
    struct timeval tv;

    char tmp_buffer[BUFFER_SIZE] = {0};

    fd_set rdfs;

    while(1) {
        i++;

        FD_ZERO(&rdfs);
        FD_SET(srvSock, &rdfs);

        tv.tv_sec = TIMER_WAIT;
        tv.tv_usec = 0;

        if((t = select(srvSock + 1, &rdfs, NULL, NULL, &tv)) == -1)
            sock_error("Erreur lors du select(), impossible de continuer");

        if(t) {
            if((n = recv(srvSock, tmp_buffer, BUFFER_SIZE, 0)) < 0)
                sock_error("Erreur lors de la réception, impossible de continuer");

            *buffer = realloc(*buffer, BUFFER_SIZE*i);
            memcpy(*buffer + (BUFFER_SIZE*(i-1)), tmp_buffer, BUFFER_SIZE);

            /* Message terminé */
            if(n < BUFFER_SIZE || tmp_buffer[BUFFER_SIZE-1] == '\0'){
                return true;
            }

        } else {

            if(*buffer)
                free(*buffer);

            return false;
        }
    }
}
