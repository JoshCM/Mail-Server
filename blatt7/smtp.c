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
#include <time.h>
#include "../blatt4/linebuffer.h"
#include "../blatt4/fileindex.h"
#include "../blatt3/database.h"
#include "../blatt1/dialog.h"
#define DB_PATH "mail.db"

int getTimeStemp(char *output)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    strftime(output, 256, "%a %b %d %T %Y", t);

    return 0;
}

int process_smtp(int infd, int outfd)
{
    DialogRec dialog[] = {
        {"HELO", "", 0, 0, NULL},
        {"MAIL FROM:", "", 0, 1, NULL},
        {"RCPT TO:", "", 1, 2, NULL},
        {"DATA", "", 2, 3, NULL},
        {"QUIT", "", 0, 0, NULL},
        {""}};

    LineBuffer *b = buf_new(infd, "\r\n");
    DBRecord databaseRecord = {"","",""};
    char *answer = calloc(1024, sizeof(char));
    char *linebuffer = calloc(1024, sizeof(char));
    char *mailfrom = calloc(1024, sizeof(char));
    char *rcptto = calloc(1024, sizeof(char));
    char *headerline = calloc(1024, sizeof(char));
    char *timestamp = calloc(1024, sizeof(char));
    char *mboxPath = calloc(1024, sizeof(char));
    char *command;
    char *param;
    int dataiscomming = 0;
    int state = 0;
    int mboxfd;
    ProlResult *res = malloc(sizeof(ProlResult));
    strcpy(answer, "220 meinkleinermailserver.de");
    write(outfd, answer, strlen(answer));
    write(outfd, "\r\n", 2);

    while (1)
    {
        buf_readline(b, linebuffer, 1024);
        if (dataiscomming)
        {
            
            if (!strcmp(linebuffer, "."))
            {
                write(mboxfd,"\n",1);
                dataiscomming = 0;
                continue;
            }
            write(mboxfd,linebuffer,strlen(linebuffer));
            write(mboxfd,"\n",1);
            continue;
        }
        *res = processLine(linebuffer, state, dialog);
        command = res->dialogrec->command;
        param = res->dialogrec->param;
        if (!strcasecmp(command, "HELO"))
        {
            strcpy(answer, "250 OK\r\n");
            write(outfd, answer, strlen(answer));
        }
        else if (!strcasecmp(command, "MAIL FROM:"))
        {
            strcpy(mailfrom, param);
            strcpy(answer, "250 OK\r\n");
            strcpy(headerline, "From ");
            strcat(headerline, mailfrom);
            strcat(headerline, "  ");
            getTimeStemp(timestamp);
            strcat(headerline, timestamp);
            write(outfd, answer, strlen(answer));
        }
        else if (!strcasecmp(command, "RCPT TO:"))
        {
            strcpy(rcptto, param);
            rcptto++;
            rcptto[strlen(rcptto)-1] = 0;
            strcpy(databaseRecord.key,rcptto);
            strcpy(databaseRecord.cat,"smtp");
            db_search(DB_PATH,0,&databaseRecord);
            strcpy(databaseRecord.key,databaseRecord.value);
            strcpy(databaseRecord.cat,"mailbox");
            db_search(DB_PATH,0,&databaseRecord);
            strcpy(mboxPath,databaseRecord.value);
            strcpy(answer, "250 OK\r\n");
            write(outfd, answer, strlen(answer));
        }
        else if (!strcasecmp(command, "DATA"))
        {
            dataiscomming = 1;
            strcpy(answer, "354 End data with <CR><LF>.<CR><LF>\r\n");
            mboxfd = open(mboxPath,O_WRONLY|O_APPEND);
            write(mboxfd,headerline,strlen(headerline));
            write(mboxfd,"\n",1);
            write(outfd, answer, strlen(answer));
        }
        else if (!strcasecmp(command, "QUIT"))
        {
            strcpy(answer, "221 Bye\r\n");
            write(outfd, answer, strlen(answer));
            break;
        }
    }

    free(mboxPath);
    free(timestamp);
    free(mailfrom);
    /*free(rcptto);*/
    free(answer);
    free(linebuffer);
    return 0;
}

int main(void)
{
    /*
    DBRecord password = {"joendhard","password","biffel"};
    DBRecord mailbox = {"joendhard","mailbox","joendhard.mbox"};
    DBRecord smtp = {"j.biffel@maildingsi.nl","smtp","joendhard"};
    
    db_put(DB_PATH,-1,&password);
    db_put(DB_PATH,-1,&mailbox);
    db_put(DB_PATH,-1,&smtp);
    */
    process_smtp(0, 1);
    return 0;
}