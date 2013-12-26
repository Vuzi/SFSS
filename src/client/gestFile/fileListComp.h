#ifndef FILELISTCOMP_H_INCLUDED
#define FILELISTCOMP_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../handling_error/herror.h"

#include "fileList.h"

/* == Structure d'un fichier == */

struct FileListComp {

	/* Fichiers ajoutés */
	FileList **addedFiles;
	int nbAddedFiles;

	/* Fichiers modifiés */
	FileList **modifiedFiles;
	int nbModifiedFiles;

	/* Fichiers supprimé */
	FileList **deletedFiles;
	int nbDeletedFiles;

};

typedef struct FileListComp FileListComp;


/* == Prototypes == */

FileListComp* newFileListComp(void);
void pushFileListCompAdded(FileListComp *f, FileList *l);
void pushFileListCompModified(FileListComp *f, FileList *l);
void pushFileListCompDeleted(FileListComp *f, FileList *l);
void deleteFileListComp(FileListComp *f);

void debugFileListComp(FileListComp *f);

#endif // FILELISTCOMP_H_INCLUDED
