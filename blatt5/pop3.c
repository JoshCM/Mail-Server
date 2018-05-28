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

int process_pop3(int infd, int outfd)
{
    DialogRec dialog = {
        {"USER","",0,1,NULL}
    };
    LineBuffer *b  = buf_new(infd,"\n");
    int test = 1;
    const int BUFFERZISE = 1024;
    char *buffer = malloc(BUFFERZISE);

    while(test>=0)
    {
        test = buf_readline(b,buffer,1024);
        printf("%s\n",buffer);
    }
    free(buffer);
    return 0;
}


int main(int argc, char const *argv[])
{
    process_pop3(0,1);
    return 0;
}

