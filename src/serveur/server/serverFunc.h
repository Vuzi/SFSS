#ifndef SERVERFUNC_H_INCLUDED
#define SERVERFUNC_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "../gestFile/gestFile.h"
#include "users/account.h"
#include "msg/msg.h"

#include "../handling_error/herror.h"

/* == Prototypes == */

void sock_error(const char* str);
void sock_error_noQuit(const char* str);

int rcvMsgFromUser(User *u, Account **a);
void sendMsgToUser(User *u, Msg *m);
void sendFile(SOCKET sock, char* filename);

void connectUser(User *u, Account **a, Msg *m);
void disconnectUser(User *u, Account **a);
void sendListToUser(User *u);
void sendSizeToUser(User *u);
void chgOptionUser(User *u, Msg *m);
void addFileUser(User *u, Msg *m);
void changeFileUser(User *u, Msg *m);
void deleteFileUser(User *u, Msg *m);
void sendFileToUser(User *u, Msg *m);

#endif // SERVERFUNC_H_INCLUDED
