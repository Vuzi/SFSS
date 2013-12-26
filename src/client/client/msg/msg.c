
#include "msg.h"

// VERSION + COMPLETE DU CLIENT

Msg *newMsg(Msg_type type, Jobj *data) {
    Msg *m = malloc(sizeof(Msg));

    m->type = type;
    m->data = data;

    return m;
}

void deleteMsg(Msg *m) {
    free(m);
}

Msg_type getMsg_Type(char *str) {
    if(strcmp(str, "connect") == 0)
        return MSG_CONNECT;
    else if(strcmp(str, "disconnect") == 0)
        return MSG_DISCONNECT;
    else if(strcmp(str, "change_option") == 0)
        return MSG_CHGOPTION;
    else if(strcmp(str, "add_file") == 0)
        return MSG_ADDFILE;
    else if(strcmp(str, "change_file") == 0)
        return MSG_CHANGEFILE;
    else if(strcmp(str, "delete_file") == 0)
        return MSG_DELETEFILE;
    else if(strcmp(str, "get_file") == 0)
        return MSG_GETFILE;
    else if(strcmp(str, "get_list") == 0)
        return MSG_GETLIST;
    else if(strcmp(str, "get_size") == 0)
        return MSG_GETSIZE;
    else if(strcmp(str, "size") == 0)
        return MSG_SIZE;
    else if(strcmp(str, "file_list") == 0)
        return MSG_LIST;
    else if(strcmp(str, "op_success") == 0)
        return MSG_OPSUCCESS;
    else if(strcmp(str, "op_fail") == 0)
        return MSG_OPFAIL;
    else
        return MSG_OTHER;
}

char* getStringMsg_Type(Msg_type type) {
    switch(type) {
        case MSG_CONNECT:
            return copy("connect");
        case MSG_DISCONNECT:
            return copy("disconnect");
        case MSG_CHGOPTION:
            return copy("change_option");
        case MSG_CHANGEFILE:
            return copy("change_file");
        case MSG_ADDFILE:
            return copy("add_file");
        case MSG_DELETEFILE:
            return copy("delete_file");
        case MSG_GETFILE:
            return copy("get_file");
        case MSG_LIST:
            return copy("file_list");
        case MSG_GETLIST:
            return copy("get_list");
        case MSG_GETSIZE:
            return copy("get_size");
        case MSG_SIZE:
            return copy("size");
        case MSG_OPSUCCESS:
            return copy("op_success");
        case MSG_OPFAIL:
            return copy("op_fail");
        case MSG_OTHER :
        default:
            return NULL;
    }
}

Msg *JobjToMsg(Jobj *j) {

    Jdata *data = NULL;
    Msg_type type;
    Msg *m = NULL;

    HERROR_WRITE(HERROR_DEBUG, "[Json to Msg] Lancement de la conversion");

    if(j) {
        /* On vérifie que le message comporte un type */
        if((data = getNextJdataStringByName(j->data, "type"))) {

            /* On récupère le type */
            type = getMsg_Type(data->d->s);

            /* On recupere le Jdata "data" */
            if((data = getNextJdataJsonByName(j->data, "data")) && data->d->j) {
                m = newMsg(type, data->d->j);
                HERROR_WRITE(HERROR_DEBUG, "[Json to Msg] Conversion terminee");
            } else {
                HERROR_WRITE(HERROR_ERROR, "[Json to Msg] Le message n'est pas correctement formatee (Absence de l'element 'data'), impossible de la traduire");
            }
        } else {
            HERROR_WRITE(HERROR_ERROR, "[Json to Msg] Le message n'est pas correctement formatee (Absence de 'type'), impossible de la traduire");
        }
    } else {
        HERROR_WRITE(HERROR_WARNING, "[Json to Msg] Message vide (nulle), aucune conversion");
    }

    return m;

}

Msg* makeConnectMsg(char* login, char* pass) {

    Jobj *j = newJobj();
    j->data = addJdataString(NULL, copy("login"), copy(login));
    addJdataString(j->data, copy("pass"), copy(pass));

    return newMsg(MSG_CONNECT, j);
}

Msg* makeDisconnectMsg(void) {

    return newMsg(MSG_DISCONNECT, newJobj());
}

Msg* makeGetListMsg(void) {

    return newMsg(MSG_GETLIST, newJobj());
}

Msg* makeGetFileMsg(Jobj *fileToGet) {

    return newMsg(MSG_GETFILE, fileToGet);
}

Msg* makeAddFileMsg(Jobj *fileToAdd) {

    return newMsg(MSG_ADDFILE, fileToAdd);
}

Msg* makeModifiedFileMsg(Jobj *fileToMod) {

    return newMsg(MSG_CHANGEFILE, fileToMod);
}

Msg* makeDeletedFileMsg(Jobj *fileToDel) {

    return newMsg(MSG_DELETEFILE, fileToDel);
}

Msg* makeSuccessMsg(Msg_type success_type, char* success_msg) {

    Jobj *j = newJobj();
    j->data = addJdataString(NULL, copy("type"), getStringMsg_Type(success_type));
    addJdataString(j->data, copy("msg"), copy(success_msg));

    return newMsg(MSG_OPSUCCESS, j);
}

Msg* makeFailMsg(Msg_type fail_type, char* fail_msg) {

    Jobj *j = newJobj();
    j->data = addJdataString(NULL, copy("type"), getStringMsg_Type(fail_type));
    addJdataString(j->data, copy("msg"), copy(fail_msg));

    return newMsg(MSG_OPFAIL, j);
}

Jobj* MsgToJobj(Msg *m) {

    Jobj *j = NULL;

    if(m) {
        j = newJobj();
        j->data = addJdataString(NULL, copy("type"), getStringMsg_Type(m->type));
        addJdataJson(j->data, copy("data"), m->data);
    }

    return j;
}

void debugMsg(Msg *m) {
    printf("Msg : type = %d | Jobj = \n", m->type);
    debugJobj(m->data, 0);
}
