#ifndef USER_H_INCLUDED
#define USER_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../../handling_error/herror.h"
#include "../../json/Json.h"
#include "../../str/strTools.h"
#include "../../gestFile/gestFile.h"

/* == Objet User == */

enum User_state {
    WAITING_MSG, WAITING_END_MSG, WAITING_FILE, WAITING_END_FILE
};

typedef enum User_state User_state;

struct User {

    /* ip & socket */
    char* ip;
    int sock;

    /* Etat utilisateur */
    User_state state;

    /* Message reçu et taille */
    char* msg;
    int msg_lenght;

    /* Fichiers en attente de l'utilisateur */
    FileList *f_tmp;
    FILE *F_tmp;

    /* Compte lié */
    struct Account *acc;

    struct User *next;
    struct User *first;

};

typedef struct User User;

#include "account.h"
#include "../server.h"

/* == Prototypes == */

User *newUser(int socket, char *ip);
User *addUser(User *u, int socket, char* ip);
User *removeUser(User *u);
void deleteUser(User *u);
User *getFirstUser(User *u);
User *getLastUser(User *u);

void clearUserBuffer(User *u);
void newUserMsg(User *u, char* msg, int msg_lenght);
void addUserMsg(User *u, char* msg, int msg_lenght);

void debugUser(User *u);

#endif // USER_H_INCLUDED
