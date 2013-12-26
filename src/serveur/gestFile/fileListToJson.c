
#include "fileListToJson.h"

Jobj* fileListToJobj(FileList *f) {

    Jobj *j = NULL, *list = NULL, *info = NULL;

    if(f) {
        HERROR_WRITE(HERROR_DEBUG, "[FileList to JSON] Debut de la conversion");

        f = f->first;

        /* Debut de la liste en JSON */
        j = newJobj();

        j->data = addJdataString(NULL, copy("type"), copy("folder_list"));
        j->data = addJdataJson(j->data, copy("list"), newJobj());
        list = j->data->d->j;
        j->data = j->data->first;

        while(f) {

            /* Pour chaque fichier */
            list->data = addJdataJson(list->data, copy("file"), newJobj());
            list->data->d->j->data = addJdataString(NULL, copy("ID"), copy(f->ID));
            addJdataString(list->data->d->j->data, copy("value"), copy(f->value));

            if(f->info) {
                list->data->d->j->data = addJdataJson(list->data->d->j->data, copy("info"), newJobj());
                info = list->data->d->j->data->d->j;

                list->data->d->j->data = list->data->d->j->data->first;

                info->data = addJdataString(NULL, copy("name"), copy(f->info->name));
                addJdataString(info->data, copy("root"), copy(f->info->root));
                addJdataInt(info->data, copy("size"), f->info->size);
                addJdataInt(info->data, copy("last_modif"), (long long int)f->info->last_modif);
            }

            f = f->next;
        }

        if(list->data)
            list->data = list->data->first;

        HERROR_WRITE(HERROR_DEBUG, "[FileList to JSON] Fin de la conversion");

    } else {
        HERROR_WRITE(HERROR_WARNING, "[FileList to JSON] Liste a convertir vide");

        /* Liste vide */
        j = newJobj();

        j->data = addJdataString(NULL, copy("type"), copy("folder_list"));
        addJdataJson(j->data, copy("list"), newJobj());
    }

    return j;
}
