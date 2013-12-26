#include <stdio.h>
#include <stdlib.h>

#include "handling_error/herror.h"
#include "json/Json.h"

#include "gestFile/gestFile.h"

#include "server/server.h"

int main()
{
    #ifdef __WIN32
        system("mode con LINES=80 COLS=160");
    #endif

    #ifdef __unix__
        printf("\e[?40h\e[?3h");
    #endif

    HERROR_START;
    _herror->current_lvl = HERROR_DEBUG;

        startServer();

    HERROR_STOP;

    exit(0);


/*
    JobjToFile(fileListToJobj(startFileListing("C:/Dropbox")), "C:/Dropbox/drop/liste1.txt", 0);
    JobjToFile(fileListToJobj(startFileListing("C:/Dropbox")), "C:/Dropbox/drop/liste2.txt", 1);
*/



    #ifdef __WIN32
    FileList *oldlist = JobjToFileList(fileToJobj("C:/Dropbox/drop/liste2.txt"));
    FileList *newlist = JobjToFileList(fileToJobj("C:/Dropbox/drop/liste1.txt"));
    #endif

    #ifdef __unix__
    FileList *oldlist = JobjToFileList(fileToJobj("/home/vuzi/Dropbox/drop/liste2.txt"));
    FileList *newlist = JobjToFileList(fileToJobj("/home/vuzi/Dropbox/drop/liste1.txt"));
    #endif

    FileListComp* f = compareFileList(newlist, oldlist);

    debugFileListComp(f);

    deleteFileListComp(f);
    deleteFileList(oldlist);
    deleteFileList(newlist);

    HERROR_STOP;

    return 0;
}
