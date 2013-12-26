#ifndef CACCOUNT_H_INCLUDED
#define CACCOUNT_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../handling_error/herror.h"
#include "../gestFile/gestFile.h"

struct C_account {

    char *login;
    char *pass;

    char *addr;
    int port;

    int sock;

};

typedef struct C_account C_account;

/* == Prototypes == */

C_account *loadClientAccount(void);
void deleteClientAccount(C_account *c);

#endif // CACCOUNT_H_INCLUDED
