#ifndef MSG_H_INCLUDED
#define MSG_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>

#include "../../json/Json.h"
#include "../../handling_error/herror.h"

/* == Objet Msg == */

enum Msg_type {
    MSG_CONNECT=0,
    MSG_DISCONNECT=1,
    MSG_CHGOPTION=2,

    MSG_ADDFILE=10,
    MSG_CHANGEFILE=11,
    MSG_DELETEFILE =12,

    MSG_GETFILE = 20,
    MSG_GETLIST = 21,
    MSG_GETSIZE = 22,

    MSG_LIST = 30,
    MSG_SIZE = 31,

    MSG_OPSUCCESS = 40,
    MSG_OPFAIL = 41,

    MSG_OTHER = 99
};

typedef enum Msg_type Msg_type;

struct Msg
{
	Msg_type type;
	Jobj *data;
};

typedef struct Msg Msg;

/* == Prototypes == */

Msg *newMsg(Msg_type type, Jobj *data);
void deleteMsg(Msg *m);

Msg *JobjToMsg(Jobj *j);
Jobj* MsgToJobj(Msg *m);
Msg_type getMsg_Type(char *str);
char* getStringMsg_Type(Msg_type type);

Msg* makeSuccessMsg(Msg_type success_type, char* success_msg);
Msg* makeFailMsg(Msg_type fail_type, char* fail_msg);
Msg* makeConnectMsg(char* login, char* pass);
Msg* makeGetListMsg(void);
Msg* makeDisconnectMsg(void);
Msg* makeGetFileMsg(Jobj *fileToGet);
Msg* makeAddFileMsg(Jobj *fileToAdd);
Msg* makeModifiedFileMsg(Jobj *fileToMod);
Msg* makeDeletedFileMsg(Jobj *fileToDel);

void debugMsg(Msg *m);

#endif // MSG_H_INCLUDED
