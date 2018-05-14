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
    return strstr(record->key, dataString) || strstr(record->cat, dataString);
}

int main(int argc, char const *argv[])
{
    const char *dbPath = "backup.db";
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
        db_list(dbPath, 1, NULL, NULL);
    }
    else if (!strcmp(argv[1], "list") && argc == 3)
    {
        db_list(dbPath, 1, besitzerCheck, argv[2]);
    }
    else if (!strcmp(argv[1], "search") && argc == 3)
    {
        db_list(dbPath, 1, subStringCheck, argv[2]);
    }
    else if (!strcmp(argv[1], "add") && argc == 4)
    {
        strcpy(record.key, argv[2]);
        strcpy(record.cat, argv[3]);
        strcpy(record.value, "Gruppenaquarium");
        index = db_search(dbPath, 0, precord);
        if (index >= 0)
        {
            db_put(dbPath, index, precord);
        }
        else if (index == -42)
        {
            db_put(dbPath, -1, precord);
        }
    }
    else if (!strcmp(argv[1], "add") && (argc == 4 || argc == 5))
    {
        strcpy(record.key, argv[2]);
        strcpy(record.cat, argv[3]);
        strcpy(record.value, "Gruppenaquarium");
        if (argc == 5)
        {
            index = db_search(dbPath, 0, precord);
            if (index >= 0)
            {
                printf("%s\n", record.value);
            }
            strcpy(record.value, argv[4]);
            db_update(dbPath, precord);
        }
        else if (argc == 4)
        {
            db_put(dbPath, -1, precord);
        }
    }
    else if (!strcmp(argv[1], "update") && argc == 4)
    {
        strcpy(record.key, argv[2]);
        index = 0;
        while (1)
        {
            index = db_search(dbPath, index, precord);

            if (index < 0)
            {
                break;
            }
            strcpy(precord->value, argv[3]);
            db_update(dbPath, precord);
            strcpy(precord->cat, "");

            index++;
        }
    }
    else if (!strcmp(argv[1], "delete"))
    {
        if (argc == 3)
        {
            strcpy(record.key, argv[2]);
            index = 0;
            while (1)
            {
                index = db_search(dbPath, index, precord);
                if (index < 0)
                {
                    break;
                }
                db_del(dbPath, index);
                strcpy(record.cat, "");
            }
        }
        else if (argc == 4)
        {
            strcpy(record.key, argv[2]);
            strcpy(record.cat, argv[3]);
            index = db_search(dbPath, 0, precord);
            db_del(dbPath, index);
        }
        else
        {
            printf("Leider kein passendes Command gefunden bitte überprüfe deine Ausgabe\n");
            return -1;
        }
    }
    return 0;
}