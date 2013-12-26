#ifndef FILELISTTOJSON_H_INCLUDED
#define FILELISTTOJSON_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../str/strTools.h"
#include "../handling_error/herror.h"
#include "fileList.h"
#include "../json/Json.h"

Jobj* fileListToJobj(FileList *f);
void fileToJdata(Jobj *list, FileList *f);

#endif // FILELISTTOJSON_H_INCLUDED
