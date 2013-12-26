
#include "server.h"

static void sock_error(const char* str) {

    #ifdef __WIN32
        HERROR_WRITE(HERROR_ERROR, "[Erreur socket] %s : '%d' de WSA", str, WSAGetLastError());
        exit(WSAGetLastError());
    #elif defined (__unix__)
        HERROR_WRITE(HERROR_ERROR, "[Erreur socket] %s : %s", str, strerror(errno));
        exit(errno);
    #endif

}

static void initW32(void)
{
#ifdef __WIN32
    WSADATA wsa;
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);
    if(err < 0)
    {
        puts("WSAStartup failed !");
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


static SOCKET startConnection(void)
{
    /* Initialisation Windows */
    initW32();

    /* Socket */
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    SOCKADDR_IN sin;

    if(sock == INVALID_SOCKET)
        sock_error("socket()");

    sin.sin_addr.s_addr = htonl(INADDR_ANY); /* Adresse */
    sin.sin_port = htons(PORT); /* Port */
    sin.sin_family = AF_INET; /* Protocole : TCP */

    /* On lie le socket */
    if(bind(sock, (SOCKADDR *)&sin, sizeof(sin)) == SOCKET_ERROR)
        sock_error("bind()");

    /* On passe en mode écoute */
    if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
        sock_error("listen()");

    HERROR_WRITE(HERROR_DEBUG, "[Lanc. serveur] Serveur en mode ecoute");

   return sock;
}


static void endConnection(void) {
    /* Fin de connexion W32 */
    endW32();
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
    	    //debugMsg(m);
    		/* Switch suivant le type de message */
    		switch(m->type) {
    			case MSG_CONNECT :
    				connectUser(u, a, m);
    				break;

    			case MSG_DISCONNECT :
    				printf("Message de déconnexion\n");
    				break;

    			case MSG_CHGOPTION :
    				printf("Message de changement d'options\n");
    				break;

    			case MSG_ADDFILE :
    				printf("Message d'ajout de fichier'\n");
    				break;

    			case MSG_CHANGEFILE :
    				printf("Message d'ajout de modification de fichier'\n");
    				break;

    			case MSG_DELETEFILE :
    				printf("Message de suppression de fichier'\n");
    				break;

    			case MSG_GETFILE :
    				printf("Message de demande de fichier'\n");
    				break;

    			case MSG_GETLIST :
    				printf("Message de demande de liste'\n");
    				break;

                case MSG_OPSUCCESS:
    				printf("Succes operation\n");
                    break;

                case MSG_OPFAIL:
    				printf("Succes operation\n");
                    break;

                case MSG_OTHER :

    			default :
    				HERROR_WRITE(HERROR_WARNING, "[Parsage mess.] Message de type inconnu, aucune operation effectuee");
    				break;
    		}
    	}

    	deleteJobj(j);
    	deleteMsg(m);

	} else {
		/* Erreur : message incorrect impossible à parser */
        HERROR_WRITE(HERROR_WARNING, "[Parsage mess.] Impossible de parser le message, aucune operation effectuee");
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

static int processMsg(User *u, Account **a) {

    int n = 0;

    /* Buffer en reception */
    char buffer[BUFFER_SIZE] = {0};

    /* Reception du message */
    if((n = recv(u->sock, buffer, BUFFER_SIZE, 0)) < 0)
    {
        sock_error("recv()");

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

                break;

            case WAITING_END_FILE :
                /* Fin/morceau de fichier */

                break;

            default :
                /* Erreur - A traiter */
                break;
        }

    }

    return n;
}

/* Programme principal serveur */
void startServer(void) {

    /* Socket du serveur */
    SOCKET sock = startConnection(); /* Fonction de lancement du serveur, ici le serveur passe en écoute */
    int max = sock;

    /* Liste des différents comptes */
    Account *accounts = NULL;

    User *users = NULL;
    User *u_cursor = NULL, *tmp = NULL;

    /* Descripteur */
    fd_set rdfs;

    bool quit = false;


    /* A ajouter : commande permettant de quitter §§§ */
    while(!quit) {

        /* On vide le descripteur */
		FD_ZERO(&rdfs);

		/* On ajoute le socket du serveur */
		FD_SET((u_int)sock, &rdfs);

        u_cursor = getFirstUser(users);

        /* On ajoute le socket de chaque User */
        while(u_cursor) {
            FD_SET((u_int)(u_cursor->sock), &rdfs);
            u_cursor = u_cursor->next;
        }

		/* On effectue le select pour lier le descripteur à la lecture sur le socket */
		if(select(max+1, &rdfs, NULL, NULL, NULL) == SOCKET_ERROR) {
            sock_error("select()");
		} else {
			/* Ici on attends un changement d'un des descripteurs */

			/* Si c'est un nouveau client qui effectue une connexion */
			if(FD_ISSET(sock, &rdfs)) {

				HERROR_WRITE(HERROR_DEBUG, "[Serveur] Nouveau client detecte");

				/* Variables du nouveau client */
				SOCKADDR_IN csin;
				size_t sinsize = sizeof csin;

				/* Socket du nouveau client */
				SOCKET csock = accept(sock, (SOCKADDR *)&csin, (socklen_t *)&sinsize);

				/* Si erreur */
				if((int)csock == SOCKET_ERROR)
                    sock_error("accept()");

				/* On ajoute le client en attente de connexion */
				users = addUser(users, csock, copy(inet_ntoa(csin.sin_addr)));

				/* Nouveau max : */
				max = (int)csock > max ? (int)csock : max;

				HERROR_WRITE(HERROR_INFO, "[Serveur] Nouveau client ajoute ( ip '%s')", users->ip);
			}
			/* Sinon c'est un client qui effectue une action */
			else {
                u_cursor = getFirstUser(users);

				/* Pour chaque Uset */
                while(u_cursor) {
                    /* S'il a envoyé des données */
                    if(FD_ISSET(u_cursor->sock, &rdfs)){
                        if(u_cursor->acc) {
                            HERROR_WRITE(HERROR_INFO, "[Serveur] Message client (ip '%s' | login '%s')", u_cursor->ip, u_cursor->acc->login);
                        } else {
                            HERROR_WRITE(HERROR_INFO, "[Serveur] Message client (ip '%s' | login '--')", u_cursor->ip);
                        }

                        /* Ici, fonction qui lit la réception du message */
                        if(processMsg(u_cursor, &accounts) == 0) {
                            /* si == 0, client déconnecté */
                            if(u_cursor->acc) {
                                HERROR_WRITE(HERROR_INFO, "[Serveur] Deconnexion client (ip '%s' | login '%s')", u_cursor->ip, u_cursor->acc->login);
                            } else {
                                HERROR_WRITE(HERROR_INFO, "[Serveur] Deconnexion client (ip '%s' | login '--')", u_cursor->ip);
                            }
                            /* On ferme la connexion */
                            closesocket(u_cursor->sock);

                            /* On l'enleve des Users */
                            tmp = u_cursor->next;
                            removeUserAndAccount(&u_cursor, &accounts);
                            users = u_cursor;
                            u_cursor = tmp;
                        } else
                            u_cursor = u_cursor->next;
                    } else
                        u_cursor = u_cursor->next;
                }
			}

		}
	}

	endConnection();

	deleteUser(users);
	deleteAccount(accounts);
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

void sendStringToUser(User *u, char *buffer) {

    /* Ici découper l'envoi en 4096 par 4096 */

   if(send(u->sock, buffer, strlen(buffer), 0) < 0)
      sock_error("send()");
}

void connectUser(User *u, Account **a, Msg *m) {

    Jdata *data = NULL;
    char *login = NULL, *pass = NULL;


    if(u->acc == NULL) {

        /* On check la présence de login et pass */
        if((data = getNextJdataStringByName(m->data->data, "login"))) {
            login = data->d->s;
            if((data = getNextJdataStringByName(m->data->data, "pass"))) {
                pass = data->d->s;

                /* On ajoute */
                if(addUserAccount(u, a, login, pass)) {
                    HERROR_WRITE(HERROR_DEBUG, "[Conn. User] Connexion effective (ip '%s' | login '%s')", u->ip, u->acc->login);
                    sendMsgToUser(u, makeSuccessMsg(MSG_CONNECT, "Connexion réussie"));
                } else {
                    HERROR_WRITE(HERROR_WARNING, "[Conn. User] Echec de la connexion (ip '%s' | login '%s')", u->ip, login);
                }

            } else {
                HERROR_WRITE(HERROR_ERROR, "[Conn. User] Aucun password specifie (ip '%s')", u->ip);
                sendMsgToUser(u, makeFailMsg(MSG_CONNECT, "Echec de la connexion : pas de password"));
            }
        } else {
            HERROR_WRITE(HERROR_ERROR, "[Conn. User] Aucun login specifie (ip '%s')", u->ip);
            sendMsgToUser(u, makeFailMsg(MSG_CONNECT, "Echec de la connexion : pas de login"));
        }

    } else {
        HERROR_WRITE(HERROR_ERROR, "[Conn. User] Un utilisateur tente de se connecter alors qu'il est deja connecte (ip '%s')", u->ip);
        sendMsgToUser(u, makeFailMsg(MSG_CONNECT, "Echec de la connexion : vous êtes déjà connecté"));
    }
}
