
#include "server.h"

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


static SOCKET startConnection(void)
{
    HERROR_WRITE(HERROR_INFO, "[Lanc. serveur] Initialisation du serveur...");

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

    HERROR_WRITE(-1, "terminee");

    /* On passe en mode écoute */
    if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
        sock_error("listen()");

   HERROR_WRITE(HERROR_INFO, "[Lanc. serveur] Serveur en mode ecoute");

   return sock;
}


static void endConnection(void) {
    /* Fin de connexion W32 */
    endW32();
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
				if(csock == SOCKET_ERROR)
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
                        if(rcvMsgFromUser(u_cursor, &accounts) == 0) {
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
