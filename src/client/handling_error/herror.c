
#include "herror.h"

herror_struct *_herror = NULL;
const char* herror_lvl_name[] = {"\n[Debug] ", "\n[Info.] ", "\n[Info.] ", "\n[Warn.] ", "\n[Error] ", "\n[Fatal] "};

FILE* HerrorStart(void) {
    _herror = malloc(sizeof(herror_struct));
    _herror->tablvl = 0;
    _herror->f = fopen(HERROR_FILENAME, "wb");
    _herror->current_lvl = HERROR_INFO;

    if(_herror->f) {
        HerrorWriteTitle(HERROR_INFO);
        HerrorPrintDate();
        HERROR_WRITE(-1, " - Debut log");
    }

    return _herror->f;
}

int HerrorStop(void) {
    if(HERROR_IS_STARTED) {
        _herror->tablvl = 0;
        HerrorWriteTitle(HERROR_INFO);
        HerrorPrintDate();
        HERROR_WRITE(-1,  " - Fin log");
        return fclose(_herror->f);
    }
    return -1;
}

void HerrorWriteTitle(int lvl) {
    int  i = 0;

    if(HERROR_IS_STARTED && lvl >= (int)_herror->current_lvl) {
        if(lvl >= 0 && lvl < HERROR_LVL_NAME_SIZE) {
            fputs(herror_lvl_name[lvl], stdout);
            fputs(herror_lvl_name[lvl], _herror->f);
        }

        for(i = 0; i < _herror->tablvl; i++) {
            putchar('|');
            putchar(' ');
            fputc('|', _herror->f);
            fputc(' ', _herror->f);
        }
    }
}

void HerrorPrintDate(void) {

    char buffer[256];
    time_t timestamp = time(NULL);

    if(HERROR_IS_STARTED) {
        strftime(buffer, sizeof(buffer), "%d-%m-%y %H:%M:%S", localtime(&timestamp));
        HERROR_WRITE(-1, "%s", buffer);
    }
}
