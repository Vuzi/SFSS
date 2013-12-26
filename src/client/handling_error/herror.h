#ifndef HERROR_H_INCLUDED
#define HERROR_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HERROR_FILENAME "out.txt"

/* == Niveau d'erreur == */
enum herror_lvl {
    HERROR_DEBUG = 0, HERROR_VINFO = 1, HERROR_INFO = 2, HERROR_WARNING = 3, HERROR_ERROR = 4, HERROR_FATAL = 5
};

typedef enum herror_lvl herror_lvl;

/* == Structure de herror == */
struct herror_struct {
    FILE* f;
    int tablvl;
    herror_lvl current_lvl;
};

typedef struct herror_struct herror_struct;

#define HERROR_LVL_NAME_SIZE 6
extern const char* herror_lvl_name[HERROR_LVL_NAME_SIZE];

extern herror_struct *_herror;

/* == Macros == */
#define HERROR_IS_STARTED (_herror && _herror->f)

#define HERROR_WRITE(lvl,msg,...) \
if(HERROR_IS_STARTED && (lvl >= (int)_herror->current_lvl || lvl == -1)) { \
    HerrorWriteTitle(lvl); \
    printf(msg, ##__VA_ARGS__); \
    fprintf(_herror->f, msg, ##__VA_ARGS__); \
    fflush(stdout); \
}

#define HERROR_INC_LVL _herror->tablvl++
#define HERROR_DEC_LVL _herror->tablvl--

#define HERROR_START HerrorStart()
#define HERROR_STOP HerrorStop()

/* == Prototypes == */

FILE* HerrorStart(void);
int HerrorStop(void);
void HerrorWriteTitle(int lvl);
void HerrorPrintDate();

#endif // HERROR_H_INCLUDED
