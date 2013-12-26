#ifndef CLIENTFUNC_H_INCLUDED
#define CLIENTFUNC_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "msg/msg.h"
#include "../client/caccount.h"

#include "../handling_error/herror.h"

/* == Prototypes == */

void sendMsgToServer(int srvSock, Msg *m);

#endif // CLIENTFUNC_H_INCLUDED
