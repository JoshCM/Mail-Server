#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "../blatt3/database.h"

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

void makeData(const char *filepath)
{
    int file_in, written;
    DBRecord rec[] = {
        {"joendhard", "password", "geheim"},
        {"joendhard", "mailbox", "joendhard.mbox"},
        {"port", "pop3", "8110"},
        {"host", "pop3", "127.0.0.1"},
        {"port", "smtp", "8582"},
        {"host", "smtp", "127.0.0.1"},
        {"j.biffel@maildingsi.nl", "smtp", "joendhard"}};
    file_in = open(filepath, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (file_in < 0)
    {
        perror("Bei Oeffnen der Eingabedatei");
        exit(2);
    }
    written = write(file_in, rec, sizeof(rec));
    if (written < 0)
    {
        perror("Beim Schreiben der Daten");
        exit(2);
    }
    db_list(filepath, 1, NULL, NULL);
    close(file_in);
}

int main(int argc, char const *argv[])
{
    const char *dbPath = "mail.db";
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
        strcpy(record.value, "");
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
    else if (!strcmp(argv[1], "reset"))
    {
        makeData(dbPath);
    }
    return 0;
}