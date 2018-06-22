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

int process_smtp(int infd, int outfd)
{
    DialogRec dialog[] = {
        {"HELO", "", 0, 0, NULL},
        {"MAIL FROM:", "", 0, 0, NULL},
        {"RCPT TO:", "", 0, 0, NULL},
        {"DATA", "", 0, 0, NULL},
        {"QUIT", "", 0, 0, NULL},
        {""}};

    LineBuffer *b = buf_new(infd, "\n");
    char *answer = calloc(255, sizeof(char));
    char *linebuffer = calloc(1024, sizeof(char));
    char *mailfrom = calloc(1024, sizeof(char));
    char *rcptto = calloc(1024, sizeof(char));
    char *command;
    char *param;
    int dataiscomming = 0;
    int state = 0;
    ProlResult *res = malloc(sizeof(ProlResult));
    strcpy(answer, "220 meinkleinermailserver.de");
    write(outfd, answer, strlen(answer));
    write(outfd, "\r\n", 2);

    while (1)
    {
        buf_readline(b, linebuffer, 1024);
        *res = processLine(linebuffer, state, dialog);
        command = res->dialogrec->command;
        param = res->dialogrec->param;
        if (dataiscomming)
        {
            if (!strcmp(linebuffer,"\r\n.\r\n"))
            {
                dataiscomming = 0;
            }
        }
        if (!strcasecmp(command, "HELO"))
        {
            strcpy(answer, "250 OK\r\n");
            write(outfd, answer, strlen(answer));
        }
        else if (!strcasecmp(command, "MAIL FROM:"))
        {
            strcpy(mailfrom, param);
            strcpy(answer, "250 OK\r\n");
            write(outfd, answer, strlen(answer));
        }
        else if (!strcasecmp(command, "RCPT TO:"))
        {
            strcpy(rcptto, param);
            strcpy(answer, "250 OK\r\n");
            write(outfd, answer, strlen(answer));
        }
        else if (!strcasecmp(command, "DATA"))
        {
            dataiscomming = 1;
            strcpy(answer, "354 End data with <CR><LF>.<CR><LF>\r\n");
            write(outfd, answer, strlen(answer));
        }
        else if (!strcasecmp(command, "QUIT"))
        {
            strcpy(answer, "221 Bye\r\n");
            write(outfd, answer, strlen(answer));
            break;
        }
    }

    free(mailfrom);
    free(rcptto);
    free(answer);
    free(linebuffer);
    return 0;
}

int main(void)
{
    process_smtp(0, 1);
    return 0;
}