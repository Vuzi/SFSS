#ifndef CLIENTFUNC_H_INCLUDED
#define CLIENTFUNC_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <utime.h>

#include "client.h"
#include "msg/msg.h"
#include "../client/caccount.h"

#include "../handling_error/herror.h"

#define TIMER_WAIT 30

/* == Prototypes == */

void sock_error(const char* str);
bool makeNewEmptyFileListFile(void);
bool getFileFromServer(C_account *usr, FileList* toRcv);
void sendMsgToServer(SOCKET srvSock, Msg *m);
void sendFileToServer(SOCKET srvSock, char* folder, FileList* toSend);
bool getSuccessResponse(SOCKET srvSock, Msg_type typeResponse, char** responseString);
Msg *getMsgResponse(SOCKET srvSock);
bool getServerResponse(SOCKET srvSock, char **buffer);

#endif // CLIENTFUNC_H_INCLUDED
