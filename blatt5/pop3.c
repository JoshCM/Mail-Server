#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "../blatt4/linebuffer.h"
#include "../blatt4/fileindex.h"
#include "../blatt3/database.h"
#include "../blatt1/dialog.h"

#define DB_PATH "mail.db"

int validate_noparam(DialogRec *d);

int process_pop3(int infd, int outfd)
{
    DialogRec dialog[] = {
        {"USER", "", 0, 1, NULL},
        {"PASS", "", 1, 2, NULL},
        {"STAT", "", 2, 2, validate_noparam},
        {""}};

    LineBuffer *b = buf_new(infd, "\n");
    int linecount = 1;
    const int BUFFERZISE = 1024;
    char *buffer = malloc(BUFFERZISE);
    char *answer = malloc(255);
    char *user;
    char *command;
    FileIndex *fi = malloc(sizeof(FileIndex));
    int state = 0;
    ProlResult *res = malloc(sizeof(ProlResult));
    DBRecord *record = malloc(sizeof(DBRecord));

    while (linecount >= 0)
    {
        linecount = buf_readline(b, buffer, 1024);
        *res = processLine(buffer, state, dialog);

        if (!res->failed)
        {
            command = res->dialogrec->command;
            if (!strcasecmp(command, "USER"))
            {
                user = res->dialogrec->param;
                write(outfd, "+OK: ", 5);
                strcpy(answer, "Username Accepted");
                write(outfd, answer, strlen(answer));
                write(outfd, "\n", 1);
            }
            else if (!strcasecmp(command, "PASS"))
            {
                strcpy(record->value, user);
                strcpy(record->cat, "password");
                db_search(DB_PATH, 0, record);
                if (strcmp(res->dialogrec->param, record->value))
                {
                    strcpy(record->value, user);
                    strcpy(record->cat, "mailbox");
                    db_search(DB_PATH, 0, record);
                    fi_new(record->value, "From");
                    write(outfd, "+OK: ", 5);
                    strcpy(answer, "Authentification successful");
                    write(outfd, answer, strlen(answer));
                    write(outfd, "\n", 1);
                }
                else
                {
                    write(outfd, "+OK: ", 5);
                    strcpy(answer, "Authentification failed");
                    write(outfd, answer, strlen(answer));
                    write(outfd, "\n", 1);
                    continue; /**skips the state change*/
                }
            }
            else if (!strcasecmp(command, "stat"))
            {
                write(outfd, "+OK: ", 5);
                sprintf(answer, "%d", fi->nEntries);
                write(outfd, answer, strlen(answer));
                write(outfd, "\n", 1);
            }

            state = res->dialogrec->nextstate;
        }
        else
        {
            write(outfd, res->info, strlen(res->info));
            write(outfd, "\n", 1);
        }
    }

    free(record);
    free(buffer);
    free(res);
    return 0;
}

int main(int argc, char const *argv[])
{
    DBRecord account = {"joendhard", "password", "biffel"};
    DBRecord mailbox = {"joendhard", "mailbox", "joendhard.mbox"};
    db_put(DB_PATH, -1, &account);
    db_put(DB_PATH, -1, &mailbox);
    process_pop3(0, 1);
    return 0;
}