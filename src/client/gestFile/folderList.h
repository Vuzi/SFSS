#ifndef FOLDERLIST_H_INCLUDED
#define FOLDERLIST_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

#include "../handling_error/herror.h"

#ifndef __WIN32
    #include <sys/types.h>
#endif

#ifdef __unix__
    #define __USE_LARGEFILE64
    #define __stat64 stat64
    #define _stat64(data1, data2) stat64(data1, data2)
#endif

#include <sys/stat.h>
#include <unistd.h>

#include "json/Json.h"

Jobj* makeDirList(char* root);
void makeDirContentList(Jobj* container, char* dir);

Jobj* makeNewJobjDir(char* root, char* name, struct __stat64* s);
Jobj* makeNewJobjFile(char* root, char* name, struct __stat64* s);

struct __stat64* getFileStat(char* dir,  char* filename);

#endif // FOLDERLIST_H_INCLUDED
