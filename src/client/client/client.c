
#include "client.h"

static void initW32(void)
{
#ifdef __WIN32
    WSADATA wsa;
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);
    if(err < 0)
    {
        HERROR_WRITE(HERROR_ERROR, "[WSAStartup] WSAStartup failed");
        exit(EXIT_FAILURE);
    }
#endif
}


static void endW32(void)
{
#ifdef __WIN32
    WSACleanup();
#endif
}


static SOCKET startClientConnection(const char *addr, int port)
{
    HERROR_WRITE(HERROR_INFO, "[Conn. serveur] Initialisation connexion...");

    /* Initialisation Windows */
    initW32();

    /* Socket */
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    SOCKADDR_IN sin;

    /* Informations de l'host */
    struct hostent *hinfo;

    if(sock == INVALID_SOCKET)
        sock_error("socket()");

    if(!(hinfo = gethostbyname(addr)))
        sock_error("Impossible de joindre le serveur");

    sin.sin_addr = *(IN_ADDR *) hinfo->h_addr; /* Adresse */
    sin.sin_port = htons(port); /* Port */
    sin.sin_family = AF_INET; /* Protocole : TCP */

    /* On connecte le socket */
    if(connect(sock, (SOCKADDR *)&sin, sizeof(sin)) == SOCKET_ERROR)
        sock_error("Impossible de se connecter (connect())");

    HERROR_WRITE(-1, "terminee");
    HERROR_WRITE(HERROR_INFO, "[Conn. serveur] Connexion effective");

    return sock;
}


static void endClientConnection(void) {
    /* Fin de connexion W32 */
    endW32();
}


/* Programme principal client */
void connectToServer(C_account *usr) {

    char *buffer = NULL;

    bool quit = false;

    Jobj *j = NULL;
    FileList *f = NULL;

    usr->sock = startClientConnection(usr->addr, usr->port);

    /* On commence par s'authentifier auprès du serveur */
    sendMsgToServer(usr->sock, makeConnectMsg(usr->login, usr->pass));

    if( getSuccessResponse(usr->sock, MSG_CONNECT, &buffer) ){
        HERROR_WRITE(HERROR_INFO, "[Connexion] Authentification effective : '%s'", buffer);
    } else {
        HERROR_WRITE(HERROR_FATAL, "[Connexion] Erreur lors de l'authentification : '%s'", buffer);
        exit(EXIT_FAILURE);
    }


    /* Ancienne liste */
    j = fileToJobj("filelist");

    /* Check chargement */
    if(!j) {
        if(!makeNewEmptyFileListFile()) {
            HERROR_WRITE(HERROR_FATAL, "[Char. liste] Impossible d'ouvrir et de modifier la liste de fichier");
            exit(EXIT_FAILURE);
        }
        HERROR_WRITE(HERROR_WARNING, "[Char. liste] Fichier absent, creation fichier vide");
        j = fileToJobj("filelist");
    }

    f = JobjToFileList(j);
    deleteJobj(j);

    while(!quit) {

        #ifdef __WIN32 /* A refaire en + propre */
          //  Sleep(usr->checkInterv*1000);
        #elif defined (__unix__)
            sleep(usr->checkInterv);
        #endif

        printf("\n> Timer");

        /* Lancement du traitement */
        startFileUpdate(usr, &f);
	}

	/* On supprime la liste */
	deleteFileList(f);

    /* On se déconnecte avant de quitter */
    sendMsgToServer(usr->sock, makeDisconnectMsg());

    if( getSuccessResponse(usr->sock, MSG_CONNECT, &buffer) ){
        HERROR_WRITE(HERROR_INFO, "[Deconnexion] Desauthentification effective : '%s'", buffer);
    } else {
        HERROR_WRITE(HERROR_FATAL, "[Deconnexion] Erreur lors de la desauthentification : '%s'", buffer);
    }

	endClientConnection();
}


//  Provisoir
void startFileUpdate(C_account *usr, FileList **oldLocal) {

    FileList *newLocal = NULL, *newOnline = NULL;
    FileListComp *comp = NULL;

    if(usr) {

        HERROR_WRITE(HERROR_INFO, "[Maj Fichiers] Lancement mise a jour liste locale");

        /* Nouvelle liste */
        newLocal = startFileListing(usr->checkFolder);

        /* Comparaison */
        comp = compareFileList(newLocal, *oldLocal);
            debugFileListComp(comp);
            getchar();

        *oldLocal = processFileListComp(usr, comp, *oldLocal);

        deleteFileList(newLocal);
        free(comp);

        HERROR_WRITE(HERROR_INFO, "[Maj Fichiers] Fin mise a jour liste locale");

        /* Comparaison avec le serveur */
        HERROR_WRITE(HERROR_INFO, "[Maj Fichiers] Lancement de la mise a jour online");

        if(getFileListFromSrv(&newOnline, usr)) {
            /* Comparasion avec la liste du serveur */
            comp = compareFileList(newOnline, *oldLocal);
            debugFileListComp(comp);
            getchar();
            *oldLocal = processFileListOnlineComp(usr, comp, *oldLocal);
            deleteFileList(newOnline);
            free(comp);

        } else {
            /* Pas de liste pas de comparaison */
            HERROR_WRITE(HERROR_ERROR, "[Maj Fichiers] Impossible de recuperer la liste des fichiers du serveur, impossible de continuer");
        }

        HERROR_WRITE(HERROR_INFO, "[Maj Fichiers] Fin de la mise a jour online");

        getchar();

    }
}

bool getFileListFromSrv(FileList **list, C_account *usr) {

    Msg *m = NULL;
    Jdata* data = NULL;

    char* buffer = NULL;
    bool ret = false;

    if(usr) {
        HERROR_WRITE(HERROR_INFO, "[Char. Liste] Lancement recuperation de la liste de fichier depuis le serveur");
        sendMsgToServer(usr->sock ,makeGetListMsg());

        if((m = getMsgResponse(usr->sock))) {

            /* On regarde le type de reponse */
            if(m->type == MSG_LIST) {
                /* On a la liste */
                if(m->data) {
                    *list = JobjToFileList(m->data);
                    HERROR_WRITE(HERROR_INFO, "[Char. Liste] Recuperation de la liste de fichier depuis le serveur terminee");
                    ret = true;
                } else
                    ret = false;

            } else if (m->type == MSG_OPFAIL) {
                /* Impossible de recup la liste */
                if(m->data) {
                    data = getNextJdataStringByName(m->data->data, "msg");
                    if(data)
                        buffer = data->d->s;
                }

                HERROR_WRITE(HERROR_WARNING, "[Char. Liste] Erreur lors de la reception de la liste de fichiers depuis le serveur : '%s'", buffer);
                ret = false;

            } else {
                /* Erreur impossible de récupérer la liste */
                HERROR_WRITE(HERROR_WARNING, "[Char. Liste] Type de message inconnu, impossible de charger la liste de fichier depuis le serveur");
                ret = false;
            }

        } else {
            /* Erreur impossible de récupérer la liste */
            HERROR_WRITE(HERROR_ERROR, "[Char. Liste] Impossible de charger la liste de fichier depuis le serveur");
            ret = false;
        }
    }

    return ret;

}


static FileList* processFileListCompToAdd(C_account *usr, FileListComp *comp, FileList *old) {
    int i = 0;
    char *buffer = NULL;

    Jobj *j = NULL;

    /* Boucle des fichiers à ajouter */
    for(i = 0; i < comp->nbAddedFiles; i++) {

        HERROR_WRITE(HERROR_INFO, "[Ajout Fic.] Lancement ajout fichier '%s/%s'", comp->addedFiles[i]->info->root, comp->addedFiles[i]->info->name);

        /* Message d'ajout du fichier */
        j = newJobj();
        fileToJdata(j, comp->addedFiles[i]);
        sendMsgToServer(usr->sock, makeAddFileMsg(j));

        if( getSuccessResponse(usr->sock, MSG_ADDFILE, &buffer) ){
            HERROR_WRITE(HERROR_DEBUG, "[Ajout Fic.] Fichier correctement ajoute : '%s', envoit du fichier...", buffer);

            /* Envoit du fichier en lui même */
            sendFileToServer(usr->sock, usr->checkFolder, comp->addedFiles[i]);

            if( getSuccessResponse(usr->sock, MSG_ADDFILE, &buffer) ){
                HERROR_WRITE(HERROR_DEBUG, "[Ajout Fic.] Fichier correctement envoye : '%s'", buffer);

                /* Ajout du fichier */
                old = addFileList(old, copy(comp->addedFiles[i]->ID), copy(comp->addedFiles[i]->value));
                old->info =  comp->addedFiles[i]->info;
                comp->addedFiles[i]->info = NULL;

            } else {

                HERROR_WRITE(HERROR_WARNING, "[Ajout Fic.] [Ajout Fic.] Erreur lors de l'envoit du fichier : '%s'", buffer);
            }

        } else {
            HERROR_WRITE(HERROR_WARNING, "[Ajout Fic.] Erreur lors de l'ajout du fichier : '%s'", buffer);
        }

        HERROR_WRITE(HERROR_DEBUG, "[Ajout Fic.] Fin ajout fichier");
    }

    return old;
}

static FileList* processFileListOnlineCompToAdd(C_account *usr, FileListComp *comp, FileList *old) {
    int i = 0;
    char *buffer = NULL;

    Jobj *j = NULL;

    /* Boucle des fichiers à télécharger */
    for(i = 0; i < comp->nbAddedFiles; i++) {

        HERROR_WRITE(HERROR_INFO, "[Telec. Fic.] Lancement telechargement nouveau fichier '%s/%s'", comp->addedFiles[i]->info->root, comp->addedFiles[i]->info->name);

        /* Message de demande du fichier */
        j = newJobj();
        fileToJdata(j, comp->addedFiles[i]);
        sendMsgToServer(usr->sock, makeGetFileMsg(j));

        if( getSuccessResponse(usr->sock, MSG_GETFILE, &buffer) ){
            HERROR_WRITE(HERROR_DEBUG, "[Telec. Fic.] Fichier correctement demande : '%s', reception du fichier...", buffer);

            /* Reception du fichier en lui même */
            if (getFileFromServer(usr, comp->addedFiles[i])) {
                HERROR_WRITE(HERROR_DEBUG, "[Telec. Fic.] Fichier correctement receptionne");

                /* Ajout du fichier */
                old = addFileList(old, copy(comp->addedFiles[i]->ID), copy(comp->addedFiles[i]->value));
                old->info =  comp->addedFiles[i]->info;
                comp->addedFiles[i]->info = NULL;

            } else {

                HERROR_WRITE(HERROR_WARNING, "[Telec. Fic.] Erreur lors de la reception du fichier");
            }

        } else {
            HERROR_WRITE(HERROR_WARNING, "[Telec. Fic.] Erreur lors de la reception du fichier : '%s'", buffer);
        }

        HERROR_WRITE(HERROR_DEBUG, "[Telec. Fic.] Fin telechargement nouveau fichier");
    }

    return old;
}


static FileList* processFileListCompToModif(C_account *usr, FileListComp *comp, FileList *old) {

    int i = 0;
    char *buffer = NULL;

    Jobj *j = NULL;

    /* Boucle des fichiers à modifier */
    for(i = 0; i < comp->nbModifiedFiles; i++) {

        HERROR_WRITE(HERROR_INFO, "[Maj Fic.] Lancement mise a jour fichier '%s/%s'", comp->modifiedFiles[i]->info->root, comp->modifiedFiles[i]->info->name);

        /* Message d'ajout du fichier */
        j = newJobj();
        fileToJdata(j, comp->modifiedFiles[i]);
        sendMsgToServer(usr->sock, makeModifiedFileMsg(j));

        if( getSuccessResponse(usr->sock, MSG_CHANGEFILE, &buffer) ){

            HERROR_WRITE(HERROR_DEBUG, "[Maj Fic.] Fichier correctement modifie : '%s', envoit du nouveau fichier...", buffer);

            /* Envoit du fichier en lui même */
            sendFileToServer(usr->sock, usr->checkFolder, comp->modifiedFiles[i]);

            if( getSuccessResponse(usr->sock, MSG_ADDFILE, &buffer) ){
                HERROR_WRITE(HERROR_DEBUG, "[Maj Fic.] Fichier correctement modifie : '%s'", buffer);

                /* Modif du fichier */
                old = getFileListByID(old, comp->modifiedFiles[i]->ID);
                free(old->value);
                old->value = copy(comp->modifiedFiles[i]->value);
                old->info =  comp->modifiedFiles[i]->info;
                comp->modifiedFiles[i]->info = NULL;

                old = old->first;

            } else {
                HERROR_WRITE(HERROR_WARNING, "[Maj Fic.] Erreur lors de l'envoit du fichier modifie : '%s'", buffer);
            }

        } else {
            HERROR_WRITE(HERROR_WARNING, "[Maj Fic.] Erreur lors de la modification du fichier : '%s'", buffer);
        }

        HERROR_WRITE(HERROR_DEBUG, "[Maj Fic.] Fin mise a jour fichier");
    }

    return old;
}

static FileList* processFileListOnlineCompToModif(C_account *usr, FileListComp *comp, FileList *old) {

    int i = 0;
    char *buffer = NULL;

    Jobj *j = NULL;

    /* Boucle des fichiers à modifier */
    for(i = 0; i < comp->nbModifiedFiles; i++) {

        HERROR_WRITE(HERROR_INFO, "[Tele Fic.] Lancement telechargement nouvelle version fichier '%s/%s'", comp->modifiedFiles[i]->info->root, comp->modifiedFiles[i]->info->name);

        /* Message de modif du fichier */
        j = newJobj();
        fileToJdata(j, comp->modifiedFiles[i]);
        sendMsgToServer(usr->sock, makeGetFileMsg(j));

        if( getSuccessResponse(usr->sock, MSG_GETFILE, &buffer) ){

            HERROR_WRITE(HERROR_DEBUG, "[Tele Fic.] Fichier correctement demande : '%s', envoit de la nouvelle version du fichier...", buffer);

            /* Envoit du fichier en lui même */
            sendFileToServer(usr->sock, usr->checkFolder, comp->modifiedFiles[i]);

            /* Reception du fichier en lui même */
            if ( getFileFromServer(usr, comp->modifiedFiles[i]) ) {
                HERROR_WRITE(HERROR_DEBUG, "[Tele Fic.] Fichier correctement receptionne");

                /* Si le fichier du serveur est + vieux, alors ajouter le fichier, et sauvegarder le fichier + recent sous un autre nom */

                /* Modif du fichier */
                old = getFileListByID(old, comp->modifiedFiles[i]->ID);
                free(old->value);
                old->value = copy(comp->modifiedFiles[i]->value);
                old->info =  comp->modifiedFiles[i]->info;
                comp->modifiedFiles[i]->info = NULL;

                old = old->first;

            } else {
                HERROR_WRITE(HERROR_WARNING, "[Tele Fic.] Erreur lors de la reception du fichier modifie");
            }

        } else {
            HERROR_WRITE(HERROR_WARNING, "[Tele Fic.] Erreur lors de la modification de fichier : '%s'", buffer);
        }

        HERROR_WRITE(HERROR_DEBUG, "[Tele Fic.] Fin telechargement nouvelle version fichier");
    }

    return old;
}


static FileList* processFileListCompToDelete(C_account *usr, FileListComp *comp, FileList *old) {

    int i = 0;
    char *buffer = NULL;

    Jobj *j = NULL;

    /* Boucle des fichiers à supprimmer */
    for(i = 0; i < comp->nbDeletedFiles; i++) {

        HERROR_WRITE(HERROR_INFO, "[Supp. Fic.] Lancement suppression fichier '%s/%s'", comp->deletedFiles[i]->info->root, comp->deletedFiles[i]->info->name);

        /* Message de suppresion du fichier */
        j = newJobj();
        fileToJdata(j, comp->deletedFiles[i]);
        sendMsgToServer(usr->sock, makeDeletedFileMsg(j));

        if( getSuccessResponse(usr->sock, MSG_DELETEFILE, &buffer) ){
            HERROR_WRITE(HERROR_DEBUG, "[Supp. Fic.] Fichier correctement supprime : '%s'", buffer);

            /* Supp du fichier */
            old = removeFileListElem(getFileListByID(old, comp->deletedFiles[i]->ID));

        } else {
            HERROR_WRITE(HERROR_WARNING, "[Supp. Fic.] Erreur lors de la suppression de fichier : '%s'", buffer);
        }

        HERROR_WRITE(HERROR_DEBUG, "[Supp. Fic.] Fin suppression fichier");
    }

    return old;
}

static FileList* processFileListOnlineCompToDelete(C_account *usr, FileListComp *comp, FileList *old) {

    int i = 0;
    char *name = NULL;

    /* Boucle des fichiers à supprimmer */
    for(i = 0; i < comp->nbDeletedFiles; i++) {

        HERROR_WRITE(HERROR_INFO, "[Supp. Fic.] Lancement suppression locale fichier '%s/%s'", comp->deletedFiles[i]->info->root, comp->deletedFiles[i]->info->name);

        /* Non du fichier */
        name = calloc(sizeof(char)*(strlen(usr->checkFolder)+1+strlen(comp->deletedFiles[i]->info->root)+1+strlen(comp->deletedFiles[i]->info->name)+1), 1);

        strcat(name, usr->checkFolder);
        strcat(name, "/");

        if(strlen(comp->deletedFiles[i]->info->root) > 0) {
            strcat(name, comp->deletedFiles[i]->info->root);
            strcat(name, "/");
        }

        strcat(name, comp->deletedFiles[i]->info->name);
        unlink(name);
        free(name);

        /* Supp. liste */
        old = removeFileListElem(getFileListByID(old, comp->deletedFiles[i]->ID));

        HERROR_WRITE(HERROR_DEBUG, "[Supp. Fic.] Fin suppression locale fichier");
    }

    return old;
}


FileList* processFileListComp(C_account *usr, FileListComp *comp, FileList *old) {

    Jobj *j = NULL;

    if(usr && comp) {

        /* Traitement de la liste */
        old = processFileListCompToAdd(usr, comp, old);
        old = processFileListCompToModif(usr, comp, old);
        old = processFileListCompToDelete(usr, comp, old);

        /* Enregistrement */
        j = fileListToJobj(old);
        JobjToFile(j, "filelist", 0);
        deleteJobj(j);

    }

    return old;
}

FileList* processFileListOnlineComp(C_account *usr, FileListComp *comp, FileList *old) {

    Jobj *j = NULL;

    if(usr && comp) {

        /* Traitement de la liste */
        old = processFileListOnlineCompToAdd(usr, comp, old);
        old = processFileListOnlineCompToModif(usr, comp, old);
        old = processFileListOnlineCompToDelete(usr, comp, old);

        /* Enregistrement */
        j = fileListToJobj(old);
        JobjToFile(j, "filelist", 0);
        deleteJobj(j);

    }

    return old;
}
