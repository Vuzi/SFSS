
#include "user.h"


User *newUser(int socket_, char* ip) {

    return addUser(NULL, socket_, ip);
}

User *addUser(User *u, int socket_, char* ip) {

    User *newU = malloc(sizeof(User));

    newU->ip = ip;
    newU->state = WAITING_MSG;
    newU->sock = socket_;
    newU->msg = NULL;
    newU->msg_lenght = 0;
    newU->f_tmp = NULL;
    newU->F_tmp = NULL;
    newU->acc = NULL;

    if(u) {
        getLastUser(u)->next = newU;
        newU->first = u->first;
    } else {
        newU->first = newU;
    }

    newU->next = NULL;

    return newU;
}

User *removeUser(User *u) {

    User *before = NULL, *after = NULL, *tmp = u;

    if(tmp) {

        tmp = tmp->first;

        while(tmp && tmp != u) {
            before = tmp;
            tmp = tmp->next;
        }


        if(u) {
            after = u->next;

            deleteUser(u);

            if(before)
                before->next = after;
            else if(after)
                after->first = after;

            if(!after && before)
                before->next = NULL;

            if(after)
                return after;
            else if(before)
                return before;
        }
    }
    return NULL;
}


User *getLastUser(User *u) {
    while(u && u->next) {
        u = u->next;
    }

    return u;
}

User *getFirstUser(User *u) {
    if(u)
        u = u->first;
    return u;
}

void deleteUser(User *u) {
    if(u) {
        free(u->ip);
        free(u);
    }
}


void clearUserBuffer(User *u) {

	if(u && u->msg) {
		if(u->msg)
			free(u->msg);
		u->msg_lenght = 0;
	}

}

void newUserMsg(User *u, char* msg, int msg_lenght) {

    u->msg = malloc(sizeof(char)*(msg_lenght));
    memcpy(u->msg, msg, msg_lenght);
    u->msg_lenght = msg_lenght;

}

void addUserMsg(User *u, char* msg, int msg_lenght) {

    /* On prepare le nouveau message */
    char *tmp = malloc(sizeof(char)*(msg_lenght+u->msg_lenght)); /* Taille = ancien morceau + nouveau */
    memcpy(tmp, u->msg, u->msg_lenght); /* Morceau 1 */
    memcpy(tmp + u->msg_lenght, msg, msg_lenght); /* Morceau 2 */

    /* On libère l'ancien */
    free(u->msg);

    /* Nouvelle taille & attribution */
    u->msg_lenght += msg_lenght;
    u->msg = tmp;

}

void debugUser(User *u) {

    if(u) {
        u = u->first;

        while(u) {
            printf("User : ip=%s | account=", u->ip);
            if(u->acc)
                printf("%s\n",u->acc->login);
            else
                printf("--\n");

            u = u->next;
        }
    }

}
