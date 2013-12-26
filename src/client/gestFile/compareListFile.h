#ifndef COMPARELISTFILE_H_INCLUDED
#define COMPARELISTFILE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../handling_error/herror.h"
#include "fileList.h"
#include "fileListComp.h"

FileListComp *compareFileList(FileList *newList, FileList *oldList);

#endif // COMPARELISTFILE_H_INCLUDED
