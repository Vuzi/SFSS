#ifndef JSONTOFILELIST_H_INCLUDED
#define JSONTOFILELIST_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../str/strTools.h"
#include "../handling_error/herror.h"
#include "fileList.h"
#include "../json/Json.h"

FileList* JobjToFileList(Jobj *j);
FileList *JdataToFileList(Jdata *data);

#endif // JSONTOFILELIST_H_INCLUDED
