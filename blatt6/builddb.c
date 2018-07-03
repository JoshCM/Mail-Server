
#include "../blatt3/database.c"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

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
    db_list(filepath,1,NULL,NULL);
    close(file_in);
}

int main(int argc, char *argv[])
{
    char *filepath = "mail.db";

    if (argc > 0)
    {
        if (strcmp(argv[1], "reset") == 0)
        {
            makeData(filepath);
        }
    }
    return 0;
}
