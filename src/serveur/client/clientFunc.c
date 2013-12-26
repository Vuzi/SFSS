
#include "clientFunc.h"

static void sendStringToServer(int srvSock, char *buffer) {

    /* Ici découper l'envoi en 4096 par 4096 A FAIRE §§*/

   if(send(srvSock, buffer, strlen(buffer), 0) < 0)
      sock_error("send()");
}


void sendMsgToServer(int srvSock, Msg *m) {

    Jobj *j = NULL;
    char *c = NULL;

    j = MsgToJobj(m);
    c = JobjToChar(j);

    deleteJobj(j);
    free(m);

    sendStringToServer(srvSock, c);

    free(c);
}
