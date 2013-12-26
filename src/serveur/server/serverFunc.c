
#include "serverFunc.h"

void sock_error(const char* str) {

    #ifdef __WIN32
        HERROR_WRITE(HERROR_ERROR, "[Erreur socket] %s : '%d' de WSA", str, WSAGetLastError());
        exit(WSAGetLastError());
    #elif defined (__unix__)
        HERROR_WRITE(HERROR_ERROR, "[Erreur socket] %s : %s", str, strerror(errno));
        exit(errno);
    #endif

}

void sock_error_noQuit(const char* str) {

    #ifdef __WIN32
        HERROR_WRITE(HERROR_ERROR, "[Erreur socket] %s : '%d' de WSA", str, WSAGetLastError());
    #elif defined (__unix__)
        HERROR_WRITE(HERROR_ERROR, "[Erreur socket] %s : %s", str, strerror(errno));
    #endif

}


static void processUserFile(User *u, char* buffer, int n) {

    char *fname = NULL, *f_finalename = NULL;

    if(u->f_tmp) {

        /* Nom temporaire (final = ID) */
        fname = malloc(sizeof(char)*(250+strlen(u->acc->login)));
        sprintf(fname, "users/%s/files/~%s_%d", u->acc->login, u->f_tmp->ID, u->sock);

        /* Début de fichier */
        if(!u->F_tmp) {
            /* On doit l'ouvrir */
            u->F_tmp = fopen(fname, "wb");
        }

        /* On ajoute le morceau */
        fwrite(buffer, n, 1, u->F_tmp);

        /* Si on arrive en fin de fichier */
        if(ftello64(u->F_tmp) >= (off64_t)u->f_tmp->info->size) {
            if(checkTmpFile(u, u->F_tmp)) {

                HERROR_WRITE(HERROR_INFO, "[Telech. fich.] Fichier '%s/%s' charge (ip '%s' | login '%s')", u->f_tmp->info->root, u->f_tmp->info->name,u->ip, u->acc->login);

                /* On ferme le fichier */
                fclose(u->F_tmp);
                u->F_tmp = NULL;

                /* On déplace le fichier tmp */
                f_finalename = malloc(sizeof(char)*(strlen(u->acc->login)+strlen(u->f_tmp->ID)+20));
                sprintf(f_finalename, "users/%s/files/%s", u->acc->login, u->f_tmp->ID);

                /* On supprime l'ancien fichier s'il existe */
                unlink(f_finalename);

                if(rename(fname, f_finalename) == 0) {
                    HERROR_WRITE(HERROR_DEBUG, "[Telech. fich.] Fichier '%s/%s' enregistre (ip '%s' | login '%s')", u->f_tmp->info->root, u->f_tmp->info->name,u->ip, u->acc->login);

                    /* On ajoute à la liste */
                    u->acc->fileList = moveFileListElem(u->acc->fileList, u->f_tmp);

                    /* Sauvegarde de la liste de fichiers & maj taille totale */
                    saveAccountFileList(u->acc);
                    u->acc->act_size = getTotalSizeFileList(u->acc->fileList);

                    /* fichier correctement envoyé */
                    sendMsgToUser(u, makeSuccessMsg(MSG_ADDFILE, "Fichier correctement enregistré"));

                } else {
                    HERROR_WRITE(HERROR_WARNING, "[Telech. fich.] Erreur lors de la sauvegarde du fichier '%s/%s' (ip '%s' | login '%s')", u->f_tmp->info->root, u->f_tmp->info->name,u->ip, u->acc->login);

                    unlink(fname);

                    /* On supprime la liste */
                    deleteFileList(u->f_tmp);

                    /* fichier erreur */
                    sendMsgToUser(u, makeFailMsg(MSG_ADDFILE, "Erreur lors de la fin de la sauvegarde du fichier"));
                }
            }
            /* Erreur avec le fichier tmp */
            else {
                HERROR_WRITE(HERROR_WARNING, "[Telech. fich.] Erreur lors du chargement du fichier '%s/%s' (ip '%s' | login '%s')", u->f_tmp->info->root, u->f_tmp->info->name, u->ip, u->acc->login);

                /* On supprime le fichier temporaire */
                fclose(u->F_tmp);
                u->F_tmp = NULL;
                unlink(fname);

                /* On supprime la liste */
                deleteFileList(u->f_tmp);

                /* fichier erreur */
                sendMsgToUser(u, makeFailMsg(MSG_ADDFILE, "Erreur lors de la fin de l'enregistrement du fichier"));
            }

            /* Plus de fichier en attente */
            u->f_tmp = NULL;

            /* Retour aux messages */
            u->state = WAITING_MSG;

        } else
            u->state = WAITING_END_FILE;

        free(fname);
    }
    /* On ne devrait pas arriver ici, mais on gère */
    else {
        if(n < BUFFER_SIZE || buffer[n-1] == EOF) {
            u->state = WAITING_MSG;
        }
    }
}

static void processUserMsg(User *u, Account **a) {

    char* bracket = NULL, *tmp = u->msg;
	Msg* m = NULL;
	Jobj *j = NULL;

	/* On parse le message */
	bracket = getBracketContent(&(tmp), '{', '}');

    j = parseToJobj(bracket);

    if(bracket)
    	free(bracket);

    if(j) {

    	m = JobjToMsg(j);

    	if(m) {

    		/* Switch suivant le type de message */
    		switch(m->type) {
    			case MSG_CONNECT :
    				connectUser(u, a, m);
    				break;

    			case MSG_DISCONNECT :
    				disconnectUser(u, a);
    				break;

    			case MSG_CHGOPTION :
    				chgOptionUser(u, m);
    				break;

    			case MSG_ADDFILE :
                    addFileUser(u, m);
    				break;

    			case MSG_CHANGEFILE :
                    changeFileUser(u, m);
    				break;

    			case MSG_DELETEFILE :
    				deleteFileUser(u, m);
    				break;

    			case MSG_GETFILE :
    				sendFileToUser(u, m);
    				break;

    			case MSG_GETLIST :
    				sendListToUser(u);
    				break;

                case MSG_GETSIZE :
                    sendSizeToUser(u);
                    break;

                case MSG_OPSUCCESS:

                case MSG_OPFAIL:

                case MSG_OTHER :

                case MSG_LIST :

                case MSG_SIZE :

    			default :
    				HERROR_WRITE(HERROR_WARNING, "[Parsage mess.] Message de type non-traite/inconnu/incorrect, aucune operation effectuee");
    				sendMsgToUser(u, makeFailMsg(MSG_OTHER, "Impossible de lire le message (Type inconnu)"));
    				break;
    		}
    	} else {
            /* Erreur : message incorrect impossible à parser */
            HERROR_WRITE(HERROR_WARNING, "[Parsage mess.] Impossible de parser le message, aucune operation effectuee");
            sendMsgToUser(u, makeFailMsg(MSG_OTHER, "Impossible de lire le message"));
    	}

    	deleteJobj(j);
    	deleteMsg(m);

	} else {
		/* Erreur : message incorrect impossible à parser */
        HERROR_WRITE(HERROR_WARNING, "[Parsage mess.] Impossible de parser le message, aucune operation effectuee");
        sendMsgToUser(u, makeFailMsg(MSG_OTHER, "Impossible de lire le message"));
	}

}

static bool makeUserMsg(User *u, char *msg, int msg_lenght) {

    bool r = false;

	/* Si on attends le début du message */
	if(u->state == WAITING_MSG) {

	    /* Nouveau msg d'user */
	    newUserMsg(u, msg, msg_lenght);

		/* Message terminé */
		if(msg_lenght < BUFFER_SIZE || msg[BUFFER_SIZE-1] == '\0') {

		    //printf("\nMessage termine ! : [%s]", u->msg);
		    r = true;
		}
		/* Morceau de message */
		else {

		    //printf("\nPremier morceau de message !");

			/* On attends la suite */
			u->state = WAITING_END_MSG;
		}

	}
	/* On attend la fin / un autre morceau */
	else if(u->state == WAITING_END_MSG) {

        //printf("\nAutre morceau de message !");

        addUserMsg(u, msg, msg_lenght);

		/* Message terminé */
		if(msg_lenght < BUFFER_SIZE || msg[BUFFER_SIZE-1] == '\0') {

		    //printf("\nMais termine quand meme !");
		    r = true;
		}
	}
	/* Erreur (techniquement) impossible */
	else {
        HERROR_WRITE(HERROR_ERROR, "[Parsage mess.] Impossible d'avoir cette erreur");
		clearUserBuffer(u);
	}

	return r;

}

int rcvMsgFromUser(User *u, Account **a) {

    int n = 0;

    /* Buffer en reception */
    char buffer[BUFFER_SIZE] = {0};

    /* Reception du message */
    if((n = recv(u->sock, buffer, BUFFER_SIZE, 0)) < 0)
    {
        sock_error_noQuit("recv()");
        return 0;

    } else if (n > 0) {


        /* Stat du message en cours */
        switch(u->state) {
            case WAITING_MSG :
                /* Debut message */

            case WAITING_END_MSG :
                /* Fin/morceau message */
                if(makeUserMsg(u, buffer, n)) {
                    processUserMsg(u, a); /* On effectue la commande message */
                    clearUserBuffer(u); /* On vide le message */
                }
                break;

            case WAITING_FILE :
                /* Debut de fichier */

            case WAITING_END_FILE :
                /* Fin/morceau de fichier */
                processUserFile(u, buffer, n);
                break;

            default :
                /* Erreur - A traiter */
                break;
        }
    }

    return n;
}


static void sendStringToUser(User *u, char *buffer) {

    while(strlen(buffer) > BUFFER_SIZE) {

       if(send(u->sock, buffer, BUFFER_SIZE, 0) < 0)
          sock_error_noQuit("send()");

        buffer += BUFFER_SIZE;
    }

   if(send(u->sock, buffer, strlen(buffer), 0) < 0)
      sock_error_noQuit("send()");
}


void sendMsgToUser(User *u, Msg *m) {

    Jobj *j = NULL;
    char *c = NULL;

    j = MsgToJobj(m);
    c = JobjToChar(j);

    deleteJobj(j);
    free(m);

    sendStringToUser(u, c);

    free(c);
}


void sendFile(SOCKET sock, char* filename) {

    FILE *f = NULL;
    char buffer[BUFFER_SIZE];
    int n = 0;

    /* Ouverture fichier */
    if((f = fopen(filename, "rb"))) {

        /* Envoit fichier */
        while((n = fread(buffer, sizeof(char), BUFFER_SIZE, f)) == BUFFER_SIZE) {
           if(send(sock, buffer, BUFFER_SIZE, 0) < 0)
              sock_error_noQuit("send()");
        }

        if(send(sock, buffer, n, 0) < 0)
            sock_error_noQuit("send()");

        fclose(f);

        HERROR_WRITE(HERROR_DEBUG, "[Envoit Fic.] Envoit fichier '%s' effectue", filename);

    } else {
        HERROR_WRITE(HERROR_ERROR, "[Envoit Fic.] Impossible d'ouvrir le fichier '%s', envoit d'un fichier vide a sa place", filename);

        buffer[0] = EOF;

        if(send(sock, buffer, 1, 0) < 0)
              sock_error_noQuit("send()");
    }
}

static void connectUserError(AccountConnectError er, User *u, char* login) {

    switch(er) {
        case ACONNECT_ALREADYCONNECTED :
            HERROR_WRITE(HERROR_ERROR, "[Conn. User] Un utilisateur tente de se connecter alors qu'il est deja connecte (ip '%s')", u->ip);
            sendMsgToUser(u, makeFailMsg(MSG_CONNECT, "Echec de la connexion : vous êtes déjà connecté"));
            break;

        case ACONNECT_NOLOGIN :
            HERROR_WRITE(HERROR_ERROR, "[Conn. User] Aucun login specifie (ip '%s')", u->ip);
            sendMsgToUser(u, makeFailMsg(MSG_CONNECT, "Echec de la connexion : pas de login"));
            break;

        case ACONNECT_NOPASS :
            HERROR_WRITE(HERROR_ERROR, "[Conn. User] Aucun password specifie (ip '%s')", u->ip);
            sendMsgToUser(u, makeFailMsg(MSG_CONNECT, "Echec de la connexion : pas de password"));
            break;

        case ACONNECT_BADLOGIN :
            HERROR_WRITE(HERROR_WARNING, "[Conn. User] Compte innexistant (Mauvais login ?) (compte '%s' | ip '%s')", login, u->ip);
            sendMsgToUser(u, makeFailMsg(MSG_CONNECT, "Echec de la connexion : login et/ou password incorrect"));
            break;

        case ACONNECT_BADPASS :
            HERROR_WRITE(HERROR_WARNING, "[Conn. User] Mauvais mot de passe (compte '%s' | ip '%s')", login, u->ip);
            sendMsgToUser(u, makeFailMsg(MSG_CONNECT, "Echec de la connexion : login et/ou password incorrect"));
            break;

        case ACONNECT_LIMITUSER :
            HERROR_WRITE(HERROR_WARNING, "[Conn. User] Limite du nombre d'utilisateur depassee (compte '%s' | ip '%s')", login, u->ip);
            sendMsgToUser(u, makeFailMsg(MSG_CONNECT, "Echec de la connexion : nombre maximum d'utilisateur du compte dépassé"));
            break;

        case ACONNECT_BADIP :
            HERROR_WRITE(HERROR_WARNING, "[Conn. User] Ip non autorisee (compte '%s' | ip '%s')", login, u->ip);
            sendMsgToUser(u, makeFailMsg(MSG_CONNECT, "Echec de la connexion : adresse ip non-autorisee"));
            break;

        case ACONNECT_NOERROR :
            HERROR_WRITE(HERROR_INFO, "[Serveur] Connexion effective (ip '%s' | login '%s')", u->ip, login);
            logAccountWrite(u, "Connexion", "Connexion au compte");
            sendMsgToUser(u, makeSuccessMsg(MSG_CONNECT, "Connexion réussie"));
            break;

        default :
            HERROR_WRITE(HERROR_INFO, "[Conn. User] Erreur inconnue lors de la connexion (ip '%s')", u->ip);
            sendMsgToUser(u, makeFailMsg(MSG_CONNECT, "Erreur inconnue lors de la connexion"));
            break;
    }

    if(ACONNECT_NOERROR != er) {
        HERROR_WRITE(HERROR_WARNING, "[Serveur] Echec de la connexion (ip '%s' | login '%s')", u->ip, login);
    }
}


void connectUser(User *u, Account **a, Msg *m) {

    Jdata *data = NULL;
    char *login = NULL, *pass = NULL;
    AccountConnectError er = ACONNECT_NOERROR;

    if(u->acc == NULL) {
        if((data = getNextJdataStringByName(m->data->data, "login"))) { /* Présence login */
            login = data->d->s;
            if((data = getNextJdataStringByName(m->data->data, "pass"))) { /* Présence pass */
                pass = data->d->s;

                er = connectUserToAccount(u, a, login, pass);

                if(er == ACONNECT_NOERROR)
                    loadAccountFileList(*a);

            } else
                er = ACONNECT_NOPASS;
        } else
            er = ACONNECT_NOLOGIN;
    } else
        er = ACONNECT_ALREADYCONNECTED;

    connectUserError(er, u, login);
}


void disconnectUser(User *u, Account **a) {

    if(u->acc != NULL) {
        HERROR_WRITE(HERROR_WARNING, "[Serveur] Deconnexion effective (ip '%s' | login '%s')", u->ip, u->acc->login);
        logAccountWrite(u, "Deconnexion", "Deconnexion du compte");

        *a = removeAccount(u->acc, u->acc->ID);
        u->acc = NULL;

        if(u->f_tmp) {
            deleteFileList(u->f_tmp);
            u->f_tmp = NULL;
        }

        if(u->F_tmp) {
            fclose(u->F_tmp);
            u->F_tmp = NULL;
        }

        sendMsgToUser(u, makeSuccessMsg(MSG_CONNECT, "Déonnexion réussie"));

    } else {
        HERROR_WRITE(HERROR_WARNING, "[Deco. User] Un utilisateur tente de se deconnecter sans etre connecte (ip '%s')", u->ip);
        sendMsgToUser(u, makeFailMsg(MSG_CONNECT, "Vous devez être connecté pour vous déconnecter"));
    }
}

void sendListToUser(User *u) {

    Jobj *j = NULL;

    if(u->acc) {
        HERROR_WRITE(HERROR_INFO, "[Demande Liste fic.] Un utilisateur demande la liste des fichiers (ip '%s' | login '%s')", u->ip, u->acc->login);
        logAccountWrite(u, "Demande liste", "Envoit de la liste de fichier");
        j = fileListToJobj(u->acc->fileList);
        sendMsgToUser(u, newMsg(MSG_LIST, j));
        HERROR_WRITE(HERROR_DEBUG, "[Demande Liste fic.] Liste envoyee (ip '%s' | login '%s')", u->ip, u->acc->login);
    } else {
        HERROR_WRITE(HERROR_WARNING, "[Demande Liste fic.] Un utilisateur tente de demander une liste des fichiers sans etre connecte (ip '%s')", u->ip);
        sendMsgToUser(u, makeFailMsg(MSG_LIST, "Vous devez être connecté pour effectuer cette commande"));
    }
}

void sendSizeToUser(User *u) {

    Jobj *j = NULL;

    if(u->acc) {
        HERROR_WRITE(HERROR_INFO, "[Demande Quota] Un utilisateur demande le quota de son compte (ip '%s' | login '%s')", u->ip, u->acc->login);
        logAccountWrite(u, "Demande quota", "Envoit des informations de quota");
        j = newJobj();
        j->data = addJdataInt(NULL, copy("max_size"), u->acc->options->max_size);
        addJdataInt(j->data, copy("act_size"), u->acc->act_size);
        addJdataInt(j->data, copy("left_size"), u->acc->options->max_size - u->acc->act_size);
        sendMsgToUser(u, newMsg(MSG_SIZE, j));
        HERROR_WRITE(HERROR_DEBUG, "[Demande Quota] Quota envoyee (ip '%s' | login '%s')", u->ip, u->acc->login);
    } else {
        HERROR_WRITE(HERROR_WARNING, "[Demande Quota] Un utilisateur tente de demander un quota sans etre connecte (ip '%s')", u->ip);
        sendMsgToUser(u, makeFailMsg(MSG_SIZE, "Vous devez être connecté pour effectuer cette commande"));
    }
}

void chgOptionUser(User *u, Msg *m) {

    Jdata *data = NULL;

    if(u->acc) {
        HERROR_WRITE(HERROR_INFO, "[Demande Ch. Options] Un utilisateur demande un changement d'option (ip '%s' | login '%s')", u->ip, u->acc->login);

        if((data = getNextJdataStringByName(m->data->data, "pass"))) { /* Présence pass */
            u->acc->pass = copy(data->d->s);
            HERROR_WRITE(HERROR_DEBUG, "[Demande Ch. Options] Changement password '%s'", u->acc->login);
            logAccountWrite(u, "Modif. Info", "Changement password");
        }

        if((data = getNextJdataIntByName(m->data->data, "multi_user"))) { /* Présence multi_user */
            logAccountWrite(u, "Modif. Info", "Changement du nombre d'utilisateur (%d à %d)", u->acc->options->multi_user, data->d->i);
            u->acc->options->multi_user = data->d->i;
            HERROR_WRITE(HERROR_DEBUG, "[Demande Ch. Options] Changement nbr utilisateurs '%s'", u->acc->login);
        }

        if((data = getNextJdataStringByName(m->data->data, "add_ip"))) { /* Présence add_ip */
            logAccountWrite(u, "Modif. Info", "Ajout d'une IP à la liste (%s)", data->d->s);
            addIpAccount_ip(u->acc->regst_ip, copy(data->d->s));
            HERROR_WRITE(HERROR_DEBUG, "[Demande Ch. Options] Ajout ip '%s'", u->acc->login);
        }

        if((data = getNextJdataStringByName(m->data->data, "remove_ip"))) { /* Présence remove_ip */
            logAccountWrite(u, "Modif. Info", "Suppresion d'une IP de la liste (%s)", data->d->s);
            removeIpAccount_ip(u->acc->regst_ip, data->d->s);
            HERROR_WRITE(HERROR_DEBUG, "[Demande Ch. Options] Suppression ip '%s'", u->acc->login);
        }

        if((data = getNextJdataStringByName(m->data->data, "reset_ip"))) { /* Présence reset_ip */
            logAccountWrite(u, "Modif. Info", "Suppresion des IPs de la liste");
            deleteAccount_ip(u->acc->regst_ip);
            u->acc->regst_ip = newAccount_ip();
            HERROR_WRITE(HERROR_DEBUG, "[Demande Ch. Options] Suppression des ips '%s'", u->acc->login);
        }

        saveAccount(u->acc);
        HERROR_WRITE(HERROR_DEBUG, "[Demande Ch. Options] Changement d'options effectue (ip '%s' | login '%s')", u->ip, u->acc->login);
        sendMsgToUser(u, makeSuccessMsg(MSG_CHGOPTION, "Changement d'options effectué avec succès"));

    } else {
        HERROR_WRITE(HERROR_WARNING, "[Demande Ch. Options] Un utilisateur tente de demander un changement d'option sans etre connecte (ip '%s')", u->ip);
        sendMsgToUser(u, makeFailMsg(MSG_CHGOPTION, "Vous devez être connecté pour effectuer cette commande"));
    }
}


static void addFileUserError(AccountFileError er, User *u) {

    switch(er) {
        case AADDFILE_NOERROR :
            HERROR_WRITE(HERROR_INFO, "[Demande Ajout fich.] Fichier ajoute en attente, attente du fichier (ip '%s' | login '%s')", u->ip, u->acc->login);
            sendMsgToUser(u, makeSuccessMsg(MSG_ADDFILE, "Fichier correctement ajouté à la liste d'attente, attente du fichier"));
            break;

        case AADDFILE_NOFILE :
            HERROR_WRITE(HERROR_WARNING, "[Demande Ajout fich.] Impossible de lire la liste de fichier (ip '%s' | login '%s')", u->ip, u->acc->login);
            sendMsgToUser(u, makeFailMsg(MSG_ADDFILE, "Erreur sur la liste des fichiers à ajouter"));
            break;

        case AADDFILE_QUOTA :
            HERROR_WRITE(HERROR_WARNING, "[Demande Ajout fich.] Quota depasse, impossible d'ajouter le fichier (ip '%s' | login '%s')", u->ip, u->acc->login);
            sendMsgToUser(u, makeFailMsg(MSG_ADDFILE, "Quota de fichier dépassé, impossible d'ajout le fichier"));
            break;

        case AADDFILE_EXIST :
            HERROR_WRITE(HERROR_WARNING, "[Demande Ajout fich.] Le fichier existe deja (ip '%s' | login '%s')", u->ip, u->acc->login);
            sendMsgToUser(u, makeFailMsg(MSG_ADDFILE, "Le fichier ajouté porte le même nom et la même racine qu'un fichier existant. Impossible de l'ajouter"));
            break;

        case AADDFILE_LOCKED :

        case AADNOCONNECT :

        default :
            HERROR_WRITE(HERROR_WARNING, "[Demande Ajout fich.] Un utilisateur tente de demander un ajout de fichier sans etre connecte (ip '%s')", u->ip);
            sendMsgToUser(u, makeFailMsg(MSG_ADDFILE, "Vous devez être connecté pour effectuer cette commande"));
            break;
    }
}


void addFileUser(User *u, Msg *m) {

    AccountFileError er = AADDFILE_NOERROR;

    if(u->acc) {
        HERROR_WRITE(HERROR_INFO, "[Demande Ajout fich.] Un utilisateur demande un ajout de fichier (ip '%s' | login '%s')", u->ip, u->acc->login);

        if((u->f_tmp = getFileListFromMsg(m->data))) {
            /* Vérification du fichier */
            if((er = checkFileToAdd(u->f_tmp, u)) == AADDFILE_NOERROR) {
                u->state = WAITING_FILE;

                logAccountWrite(u, "Ajout fichier", "Ajout du fichier '%s/%s'", u->f_tmp->info->root, u->f_tmp->info->name);
            }
        } else
            er = AADDFILE_NOFILE;

    } else
        er = AADNOCONNECT;

    addFileUserError(er, u);
}

static void changeFileUserError(AccountFileError er, User *u) {

    switch(er) {
        case AADDFILE_NOERROR :
            HERROR_WRITE(HERROR_INFO, "[Demande Modif. fich.] Fichier modifie en attente, attente du fichier (ip '%s' | login '%s')", u->ip, u->acc->login);
            sendMsgToUser(u, makeSuccessMsg(MSG_CHANGEFILE, "Fichier correctement ajouté à la liste d'attente de modification, attente du fichier"));
            break;

        case AADDFILE_NOFILE :
            HERROR_WRITE(HERROR_WARNING, "[Demande Modif. fich.] Impossible de lire la liste de fichier (ip '%s' | login '%s')", u->ip, u->acc->login);
            sendMsgToUser(u, makeFailMsg(MSG_CHANGEFILE, "Erreur sur la liste des fichiers à modifier"));
            break;

        case AADDFILE_QUOTA :
            HERROR_WRITE(HERROR_WARNING, "[Demande Modif. fich.] Quota depasse, impossible de modifier le fichier (ip '%s' | login '%s')", u->ip, u->acc->login);
            sendMsgToUser(u, makeFailMsg(MSG_CHANGEFILE, "Quota de fichier dépassé, impossible de modifier le fichier"));
            break;

        case AADDFILE_EXIST :
            HERROR_WRITE(HERROR_WARNING, "[Demande Modif. fich.] Le fichier n'existe pas (ip '%s' | login '%s')", u->ip, u->acc->login);
            sendMsgToUser(u, makeFailMsg(MSG_CHANGEFILE, "Le fichiers à modifier n'existe pas. Impossible de le modifier"));
            break;

        case AADDFILE_LOCKED :
            HERROR_WRITE(HERROR_WARNING, "[Demande Modif. fich.] Le fichier est en cours d'operation par un autre utilisateur (ip '%s' | login '%s')", u->ip, u->acc->login);
            sendMsgToUser(u, makeFailMsg(MSG_CHANGEFILE, " Le fichier est en cours d'opération par un autre utilisateur. Impossible de le modifier"));
            break;

        case AADNOCONNECT :

        default :
            HERROR_WRITE(HERROR_WARNING, "[Demande Modif. fich.] Un utilisateur tente de demander une modification de fichier sans etre connecte (ip '%s')", u->ip);
            sendMsgToUser(u, makeFailMsg(MSG_CHANGEFILE, "Vous devez être connecté pour effectuer cette commande"));
            break;
    }
}

void changeFileUser(User *u, Msg *m) {

    AccountFileError er = AADDFILE_NOERROR;

    if(u->acc) {
        HERROR_WRITE(HERROR_INFO, "[Demande Ajout fich.] Un utilisateur demande une modification de fichier (ip '%s' | login '%s')", u->ip, u->acc->login);

        if((u->f_tmp = getFileListFromMsg(m->data))) {
            /* Vérification du premier fichier */
            if((er = checkFileToChange(u->f_tmp, u)) == AADDFILE_NOERROR) {
                u->state = WAITING_FILE;

                logAccountWrite(u, "Modification fichier", "Modification du fichier '%s/%s'", u->f_tmp->info->root, u->f_tmp->info->name);
            }
        } else
            er = AADDFILE_NOFILE;

    } else
        er = AADNOCONNECT;

    changeFileUserError(er, u);
}

static void deleteFileUserError(AccountFileError er, User *u) {

    switch(er) {
        case AADDFILE_NOERROR :
            HERROR_WRITE(HERROR_INFO, "[Demande Supp. fich.] Fichier correctement supprime (ip '%s' | login '%s')", u->ip, u->acc->login);
            sendMsgToUser(u, makeSuccessMsg(MSG_DELETEFILE, "Fichier correctement supprimé"));
            break;

        case AADDFILE_NOFILE :
            HERROR_WRITE(HERROR_WARNING, "[Demande Supp. fich.] Impossible de lire la liste de fichier (ip '%s' | login '%s')", u->ip, u->acc->login);
            sendMsgToUser(u, makeFailMsg(MSG_DELETEFILE, "Erreur sur la liste des fichiers à supprimer"));
            break;

        case AADDFILE_EXIST :
            HERROR_WRITE(HERROR_WARNING, "[Demande Supp. fich.] Le fichier n'existe pas (ip '%s' | login '%s')", u->ip, u->acc->login);
            sendMsgToUser(u, makeFailMsg(MSG_DELETEFILE, "Le fichiers à supprimer n'existe pas. Impossible de le supprimer"));
            break;

        case AADDFILE_LOCKED :
            HERROR_WRITE(HERROR_WARNING, "[Demande Supp. fich.] Le fichier est en cours d'operation par un autre utilisateur (ip '%s' | login '%s')", u->ip, u->acc->login);
            sendMsgToUser(u, makeFailMsg(MSG_DELETEFILE, " Le fichier est en cours d'opération par un autre utilisateur. Impossible de le supprimer"));
            break;

        case AADDFILE_QUOTA :
        case AADNOCONNECT :

        default :
            HERROR_WRITE(HERROR_WARNING, "[Demande Supp. fich.] Un utilisateur tente de demander une suppression de fichier sans etre connecte (ip '%s')", u->ip);
            sendMsgToUser(u, makeFailMsg(MSG_DELETEFILE, "Vous devez être connecté pour effectuer cette commande"));
            break;
    }
}

void deleteFileUser(User *u, Msg *m) {

    AccountFileError er = AADDFILE_NOERROR;

    if(u->acc) {
        HERROR_WRITE(HERROR_INFO, "[Demande Supp. fich.] Un utilisateur demande une suppression de fichier (ip '%s' | login '%s')", u->ip, u->acc->login);

        if((u->f_tmp = getFileListFromMsg(m->data))) {
            logAccountWrite(u, "Suppression fichier", "Suppression du fichier '%s/%s'", u->f_tmp->info->root, u->f_tmp->info->name);
            er = deleteFileInFileList(u->f_tmp, u);

            /* Sup. fichier temp. */
            deleteFileList(u->f_tmp);
            u->f_tmp = NULL;
        } else
            er = AADDFILE_NOFILE;

    } else
        er = AADNOCONNECT;

    deleteFileUserError(er, u);
}

static void sendFileToUserError(AccountFileError er, User *u) {

    switch(er) {
        case AADDFILE_NOERROR :
            HERROR_WRITE(HERROR_DEBUG, "[Demande fich.] Debut envoit du fichier (ip '%s' | login '%s')", u->ip, u->acc->login);
            sendMsgToUser(u, makeSuccessMsg(MSG_GETFILE, "Le fichier demandé va être envoyé"));
            break;

        case AADDFILE_NOFILE :
            HERROR_WRITE(HERROR_WARNING, "[Demande fich.] Impossible de recuperer le fichier demande (ip '%s' | login '%s')", u->ip, u->acc->login);
            sendMsgToUser(u, makeFailMsg(MSG_GETFILE, "Impossible de recupérer le fichier demandé"));
            break;

        case AADDFILE_EXIST :
            HERROR_WRITE(HERROR_WARNING, "[Demande fich.] Le fichier a envoyer n'existe pas (ip '%s' | login '%s')", u->ip, u->acc->login);
            sendMsgToUser(u, makeFailMsg(MSG_GETFILE, "Le fichiers à envoyer n'existe pas. Impossible de l'envoyer"));
            break;

        case AADDFILE_LOCKED :
            HERROR_WRITE(HERROR_WARNING, "[Demande fich.] Le fichier est en cours d'operation par un autre utilisateur (ip '%s' | login '%s')", u->ip, u->acc->login);
            sendMsgToUser(u, makeFailMsg(MSG_GETFILE, " Le fichier est en cours d'opération par un autre utilisateur. Impossible de l'envoyer"));
            break;

        case AADDFILE_QUOTA :
        case AADNOCONNECT :

        default :
            HERROR_WRITE(HERROR_WARNING, "[Demande fich.] Un utilisateur tente de demander une suppression de fichier sans etre connecte (ip '%s')", u->ip);
            sendMsgToUser(u, makeFailMsg(MSG_GETFILE, "Vous devez être connecté pour effectuer cette commande"));
            break;
    }
}

void sendFileToUser(User *u, Msg *m) {

    AccountFileError er = AADDFILE_NOERROR;
    char *filename = NULL;

    if(u->acc) {
        HERROR_WRITE(HERROR_INFO, "[Demande fich.] Un utilisateur demande un fichier (ip '%s' | login '%s')", u->ip, u->acc->login);

        if((u->f_tmp = getFileListFromMsg(m->data)))
            er = checkFileToChange(u->f_tmp, u);
        else
            er = AADDFILE_NOFILE;

    } else
        er = AADNOCONNECT;

    sendFileToUserError(er, u);

    if(er == AADDFILE_NOERROR) {
        /* Si aucune erreur, envoit du fichier */
        filename = malloc(sizeof(char)*(strlen(u->acc->login)+strlen(u->f_tmp->ID)+20));
        sprintf(filename, "users/%s/files/%s", u->acc->login, u->f_tmp->ID);
        sendFile(u->sock, filename);
        free(filename);
    }
}
