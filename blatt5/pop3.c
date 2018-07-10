 #define _POSIX_SOURCE 1
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include "../blatt4/linebuffer.h"
#include "../blatt4/fileindex.h"
#include "../blatt3/database.h"
#include "../blatt1/dialog.h"

#define DB_PATH "mail.db"

void myinthandler(int sig)
{
    exit(0);
}

int lockFile(char *path)
{
    int fd, prevpid;
    int lambdahalbe;
    char *pid = calloc(1024, sizeof(char));
    char *prevPidBuf = calloc(1024, sizeof(char));

    sprintf(pid, "%d", getpid());
    fd = open(path, O_RDWR | O_CREAT | O_EXCL, 0644);
    if (fd >= 0)
    {
        write(fd, pid, strlen(pid));
        close(fd);
    }
    else
    {
        fd = open(path, O_RDONLY);
        lambdahalbe = read(fd, prevPidBuf, 1024);
        prevPidBuf[lambdahalbe] = 0;
        prevpid = atoi(prevPidBuf);
        if (kill(prevpid, 0) != 0)
        {
            close(fd);
            unlink(path);
            free(pid);
            free(prevPidBuf);
            return lockFile(path);
        }
        else
        {
            fd = -1;
        }
    }
    free(pid);
    free(prevPidBuf);
    return fd >= 0;
}

int freeFile(char *path)
{
    return (unlink(path) == 0);
}

int validate_noparam(DialogRec *d);

int process_pop3(int infd, int outfd)
{
    DialogRec dialog[] = {
        {"USER", "", 0, 1, NULL},
        {"PASS", "", 1, 2, NULL},
        {"STAT", "", 2, 2, validate_noparam},
        {"LIST", "", 2, 2, NULL},
        {"RETR", "", 2, 2, NULL},
        {"NOOP", "", 2, 2, validate_noparam},
        {"DELE", "", 2, 2, NULL},
        {"RSET", "", 2, 2, validate_noparam},
        {"QUIT", "", 2, 0, validate_noparam},
        {""}};

    LineBuffer *b = buf_new(infd, "\r\n");
    LineBuffer *fib;
    int linecount = 1;
    const int BUFFERZISE = 1024;
    char *buffer = malloc(BUFFERZISE);
    char *answer = malloc(255);
    char *lockpath = malloc(255);
    char *user;
    char *command;
    FileIndex *fi = malloc(sizeof(FileIndex));
    int state = 0;
    ProlResult *res = malloc(sizeof(ProlResult));
    DBRecord *record = malloc(sizeof(DBRecord));
    FileIndexEntry *fiePointer;
    int requestedIndex = 1;
    int i, indexToDelete, totalFileSize = 0, undeletedFie = 0;
    signal(SIGINT, myinthandler);

    strcpy(answer, "+OK POP3 ready.\r\n");
    write(outfd, answer, strlen(answer));
    while (1)
    {
        linecount = buf_readline(b, buffer, 1024);
        if (linecount < 0)
        {
            break;
        }

        *res = processLine(buffer, state, dialog);

        if (!res->failed)
        {
            command = res->dialogrec->command;
            if (!strcasecmp(command, "USER"))
            {
                user = res->dialogrec->param;
                write(outfd, "+OK\r\n", 5);
            }
            else if (!strcasecmp(command, "PASS"))
            {
                strcpy(lockpath, "/tmp/");
                strcpy(record->value, user);
                strcpy(record->cat, "password");
                db_search(DB_PATH, 0, record);
                if (!strcmp(res->dialogrec->param, record->value))
                {
                    strcpy(record->value, user);
                    strcpy(record->cat, "mailbox");
                    db_search(DB_PATH, 0, record);
                    strcat(lockpath, user);
                    strcat(lockpath, ".lock");
                    if (!lockFile(lockpath))
                    {
                        write(outfd, "-ERR ", 5);
                        strcpy(answer, "You are already logged in");
                        write(outfd, answer, strlen(answer));
                        write(outfd, "\r\n", 2);
                        state = 0;
                        continue;
                    }
                    fi = fi_new(record->value, "From ");
                    write(outfd, "+OK ", 4);
                    strcpy(answer, "Authentification successful");
                    write(outfd, answer, strlen(answer));
                    write(outfd, "\r\n", 2);
                }
                else
                {
                    write(outfd, "+OK ", 4);
                    strcpy(answer, "Authentification failed");
                    write(outfd, answer, strlen(answer));
                    write(outfd, "\r\n", 2);
                    state = 0;
                    continue; /**skips the state change*/
                }
            }
            else if (!strcasecmp(command, "stat"))
            {
                write(outfd, "+OK ", 4);

                if (!fi->totalSize)
                {
                    sprintf(answer, "%d ", 0);
                    write(outfd, answer, strlen(answer));
                    sprintf(answer, "%d", fi->totalSize);
                    write(outfd, answer, strlen(answer));
                }
                else
                {
                    fiePointer = fi->entries;
                    while (fiePointer)
                    {
                        if (!fiePointer->del_flag)
                        {
                            totalFileSize += fiePointer->size + fiePointer->lines;
                            undeletedFie++;
                        }
                        fiePointer = fiePointer->next;
                    }
                    sprintf(answer, "%d ", undeletedFie);
                    write(outfd, answer, strlen(answer));
                    sprintf(answer, "%d", totalFileSize);
                    write(outfd, answer, strlen(answer));
                }
                write(outfd, "\r\n", 2);
                totalFileSize = 0;
                undeletedFie = 0;
            }
            else if (!strcasecmp(command, "list"))
            {
                if (!strcmp(res->dialogrec->param, ""))
                {
                    write(outfd, "+OK ", 4);
                    strcpy(answer, "scan list follows");
                    write(outfd, answer, strlen(answer));
                    write(outfd, "\r\n", 2);
                    fiePointer = fi->entries;
                    while (fiePointer)
                    {
                        if (!fiePointer->del_flag)
                        {
                            sprintf(answer, "%d ", fiePointer->nr);
                            write(outfd, answer, strlen(answer));
                            sprintf(answer, "%d", fiePointer->size + fiePointer->lines);
                            write(outfd, answer, strlen(answer));
                            write(outfd, "\r\n", 2);
                        }
                        fiePointer = fiePointer->next;
                    }
                    write(outfd, ".\r\n", 3);
                }
                else
                {
                    requestedIndex = atoi(res->dialogrec->param);
                    fiePointer = fi_find(fi, requestedIndex);

                    write(outfd, "+OK ", 4);
                    sprintf(answer, "%d ", fiePointer->nr);
                    write(outfd, answer, strlen(answer));
                    sprintf(answer, "%d", fiePointer->size + fiePointer->lines);
                    write(outfd, answer, strlen(answer));
                    write(outfd, "\r\n", 2);
                    fiePointer = fiePointer->next;
                }
            }
            else if (!strcasecmp(command, "RETR"))
            {
                requestedIndex = atoi(res->dialogrec->param);
                fiePointer = fi_find(fi, requestedIndex);
                fib = buf_new(open(fi->filepath, O_RDONLY), "\n");

                buf_seek(fib, fiePointer->seekpos);

                strcpy(answer, "+OK ");
                write(outfd, answer, strlen(answer));
                sprintf(answer, "%d ", fiePointer->size);
                write(outfd, answer, strlen(answer));
                strcpy(answer, "octets\r\n");
                write(outfd, answer, strlen(answer));

                for (i = 0; i < fiePointer->lines; i++)
                {
                    buf_readline(fib, buffer, BUFFERZISE);
                    write(outfd, buffer, strlen(buffer));
                    write(outfd, "\r\n", 2);
                }

                write(outfd, ".\r\n", 3);
            }
            else if (!strcasecmp(command, "NOOP"))
            {
                strcpy(answer, "+OK ");
                write(outfd, answer, strlen(answer));
                write(outfd, "\r\n", 2);
            }
            else if (!strcasecmp(command, "DELE"))
            {
                indexToDelete = atoi(res->dialogrec->param);
                fiePointer = fi_find(fi, indexToDelete);
                fiePointer->del_flag = 1;
                strcpy(answer, "+OK ");
                write(outfd, answer, strlen(answer));
                strcpy(answer, "mail deleted");
                write(outfd, answer, strlen(answer));
                write(outfd, "\r\n", 2);
            }
            else if (!strcasecmp(command, "RSET"))
            {
                fiePointer = fi->entries;

                while (fiePointer != NULL)
                {
                    fiePointer->del_flag = 0;
                    fiePointer = fiePointer->next;
                }

                strcpy(answer, "+OK ");
                write(outfd, answer, strlen(answer));
                strcpy(answer, "restored deleted mails");
                write(outfd, answer, strlen(answer));
                write(outfd, "\r\n", 2);
            }
            else if (!strcasecmp(command, "QUIT"))
            {
                fi_compactify(fi);
                strcpy(answer, "+OK session ended\r\n");
                write(outfd, answer, strlen(answer));
                freeFile(lockpath);
                break;
            }
            state = res->dialogrec->nextstate;
        }
        else
        {
            write(outfd, "-ERR ", strlen("-ERR "));
            write(outfd, "\r\n", 2);
        }
    }

    free(record);
    free(buffer);
    free(res);
    fi_dispose(fi);
    signal(SIGINT, SIG_DFL);
    return 0;
}

/**
int main(int argc, char const *argv[])
{

    DBRecord account = {"joendhard", "password", "biffel"};
    DBRecord mailbox = {"joendhard", "mailbox", "joendhard.mbox"};
    db_put(DB_PATH, -1, &account);
    db_put(DB_PATH, -1, &mailbox);
    
    process_pop3(0, 1);
    return 0;
}
**/
