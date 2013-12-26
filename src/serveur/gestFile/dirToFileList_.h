#ifndef DIRTOFILELIST_H_INCLUDED
#define DIRTOFILELIST_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

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

#include "../str/strTools.h"
#include "../handling_error/herror.h"
#include "fileList.h"
#include "../md5/md5.h"

FileList* startFileListing(char* dir);
FileList* getFileList(char *dir, FileList *f);
FileList *makeFileListFromStat(FileList *f, char* name, char* dir, char* filename, struct __stat64* s);


#endif // DIRTOFILELIST_H_INCLUDED
