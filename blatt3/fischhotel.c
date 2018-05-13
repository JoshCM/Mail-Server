#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "database.h"

int besitzerCheck(DBRecord *record, const void *data)
{
    char *dataString = (char *)data;
    return !strcmp(record->key, dataString);
}

int subStringCheck(DBRecord *record, const void *data)
{
    char *dataString = (char *)data;
    return strstr(record->key, dataString) || strstr(record->key, dataString);
}

int main(int argc, char const *argv[])
{
    DBRecord record = {"", "", ""};
    DBRecord *precord = &record;
    int index;

    if (argc < 2)
    {
        printf("Zu wenige Argumente\n");
        return -1;
    }
    if (!strcmp(argv[1], "list") && argc == 2)
    {
        db_list("backup.db", 1, NULL, NULL);
    }
    else if (!strcmp(argv[1], "list") && argc == 3)
    {
        db_list("backup.db", 1, besitzerCheck, argv[2]);
    }
    else if (!strcmp(argv[1], "search") && argc == 3)
    {
        db_list("backup.db", 1, subStringCheck, argv[2]);
    }
    else if (!strcmp(argv[1], "add") && argc == 4)
    {
        strcpy(record.key, argv[2]);
        strcpy(record.cat, argv[3]);
        strcpy(record.value, "Gruppenaquarium");
        index = db_search("backup.db", 0, precord);
        if (index >= 0)
        {
            db_put("backup.db", index, precord);
        }
        else if (index == -42)
        {
            db_put("backup.db", -1, precord);
        }
    }
    else if (!strcmp(argv[1], "add") && (argc == 4 || argc == 5))
    {
        strcpy(record.key, argv[2]);
        strcpy(record.cat, argv[3]);
        strcpy(record.value, "Gruppenaquarium");
        if (argc == 5)
        {
            index = db_search("backup.db", 0, precord);
            if (index >= 0)
            {
                printf("%s", record.value);
            }
            strcpy(record.value, argv[4]);
            db_update("backup.db", precord);
        }
        else if (argc == 4)
        {
            db_put("backup.db", -1, precord);
        }
    }
    return 0;
}