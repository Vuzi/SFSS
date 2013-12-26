#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#ifdef __WIN32

    #include <winsock2.h>
    typedef int socklen_t;

#elif defined (__unix__)

    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h> /* close */
    #include <netdb.h> /* gethostbyname */
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close(s)
    typedef int SOCKET;
    typedef struct sockaddr_in SOCKADDR_IN;
    typedef struct sockaddr SOCKADDR;
    typedef struct in_addr IN_ADDR;

#endif

/* Port de connexion */
#define PORT	 	1992

/* Nombre max de clients */
#define MAX_CLIENTS 100

/* Taille buffer */
#define BUFFER_SIZE 8192

#include "users/account.h"
#include "msg/msg.h"
#include "serverFunc.h"

#include "../handling_error/herror.h"

/* == Prototypes == */

void startServer(void);

#endif // SERVER_H_INCLUDED
