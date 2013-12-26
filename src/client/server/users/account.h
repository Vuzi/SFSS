#ifndef ACCOUNT_H_INCLUDED
#define ACCOUNT_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifdef __WIN32
    #define fftello(args) tello64(args)
#elif defined (__unix__)
    #define _FILE_OFFSET_BITS 64
#endif

#include "../../handling_error/herror.h"
#include "../../str/strTools.h"
#include "../../json/Json.h"
#include "../../gestFile/gestFile.h"


/* == Objet Account == */

/* Représente les options du compte, qui sont chargée avec le compte,
   et enregistré à chaque modification */
struct Account_options
{
	unsigned int multi_user; /* Nombre maximum d'utilisateurs simultanés, 0 = aucune limite */
	unsigned long long int max_size; /* Quota maximum utilisable pas le compte, 0 = aucun quota */
};

typedef struct Account_options Account_options;

/* Liste d'ip autorisées */
struct Account_ip
{
	int nb_ip; /* Nombre d'ip */
	char** ip; /* Liste d'ip */
};

typedef struct Account_ip Account_ip;

/* Compte */
struct Account {

    /* ID d'ajout */
    int ID;

    /* login & pass */
    char *login;
    char *pass;

    /* Fichier de log du compte */
    FILE* log;

    /* Nombre actuel d'user connecté & taille actuel des fichiers stockés */
    unsigned int nb_user;
    unsigned long long int act_size;

    /* Liste de fichiers */
    FileList *fileList;

    /* Options & IP */
    struct Account_options *options;
    struct Account_ip *regst_ip;

    struct Account *next;
    struct Account *first;
};

typedef struct Account Account;

enum AccountConnectError {
    ACONNECT_NOERROR, ACONNECT_BADLOGIN, ACONNECT_NOLOGIN, ACONNECT_NOPASS, ACONNECT_ALREADYCONNECTED, ACONNECT_BADPASS, ACONNECT_BADIP, ACONNECT_LIMITUSER
};

typedef enum AccountConnectError AccountConnectError;

enum AccountFileError {
    AADDFILE_NOERROR, AADDFILE_NOFILE, AADDFILE_QUOTA, AADDFILE_EXIST, AADNOCONNECT, AADDFILE_LOCKED
};

typedef enum AccountFileError AccountFileError;

#include "user.h"

/* == Prototypes == */

Account* newAccount(char* login, char* pass);
Account* addAccount(Account *a, char* login, char* pass);
Account *removeAccount(Account *a, int ID);
void deleteAccount(Account *u);
Account* getLastAccount(Account *a);
void accountAddUser(Account *a, User *u);
void loadAccountFileList(Account *a);
Account* JobjToAccount(Jobj *j);
Jobj* accountToJobj(Account *a);
void accountRemoveUser(Account *a, int u);

AccountConnectError connectUserToAccount(User *u, Account **a, char* login, char* pass);
AccountConnectError getAndTestAccountFromFile(Account **a, char *login, char *pass, char* ip);
void removeUserAndAccount(User **u, Account **a);

FileList *getFileListFromMsg(Jobj *j);
bool checkTmpFile(User *u, FILE* F);
AccountFileError checkFileToChange(FileList *f, User *u);
AccountFileError checkFileToAdd(FileList *f, User *u);
AccountFileError deleteFileInFileList(FileList *f, User *u);

Account_options *newAccount_options(unsigned int multi_user, unsigned long long int max_size);

Account_ip *newAccount_ip(void);
void addIpAccount_ip(Account_ip *a, char *ip);
void removeIpAccount_ip(Account_ip *a, char *ip);
void deleteAccount_ip(Account_ip *u);
bool isInIpAccount_ip(Account_ip *a, char *ip);

void saveAccount(Account *a);
void saveAccountFileList(Account *a);

void debugAccount(Account *a);


#endif // ACCOUNT_H_INCLUDED
