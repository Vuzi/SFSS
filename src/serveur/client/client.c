
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


static void sock_error(const char* str) {

    #ifdef __WIN32
        HERROR_WRITE(HERROR_ERROR, "[Erreur socket] %s : '%d' de WSA", str, WSAGetLastError());
        exit(WSAGetLastError());
    #elif defined (__unix__)
        HERROR_WRITE(HERROR_ERROR, "[Erreur socket] %s : %s", str, strerror(errno));
        exit(errno);
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


/* Programme principal serveur */
void connectToServer(C_account *usr) {

    /* Socket du serveur */
    SOCKET sock = startClientConnection(usr->addr, usr->port);
    int max = sock;

    /* Descripteur */
    fd_set rdfs;

    bool quit = false;



    /* A ajouter : commande permettant de quitter §§§ */
    while(!quit) {


	}

	endClientConnection();
}
