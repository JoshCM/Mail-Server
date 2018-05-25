#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "linebuffer.h"
#include "fileindex.h"

FileIndex *fi_new(const char *filepath, const char *separator)
{
    int fd = open(filepath, O_RDWR | O_CREAT, 0640);
    int sectionStart = 0;
    int fieIndex = 1;
    FileIndex *newFI = malloc(sizeof(FileIndex));
    FileIndexEntry *newFIE = malloc(sizeof(FileIndexEntry));
    LineBuffer *b = buf_new(fd, "\n");
    char line[LINEBUFFERSIZE];
    char *linepointer = line;

    newFI->filepath = filepath;
    newFI->entries = newFIE;

    newFIE->nr = fieIndex;

    while (1)
    {
        sectionStart = buf_readline(b, linepointer, LINEBUFFERSIZE);
        if (sectionStart < 0)
        {
            break;
        }

        if (!strcmp(linepointer, separator))
        {
            sectionStart = buf_readline(b, linepointer, LINEBUFFERSIZE);
            newFIE->seekpos = sectionStart;
            newFIE->lines += 1;
        }
        else if (!strcmp(linepointer, ""))
        {
            sectionStart = buf_readline(b, linepointer, LINEBUFFERSIZE);
            if (!strcmp(linepointer, separator))
            {
                newFIE->next = malloc(sizeof(FileIndexEntry));
                newFIE->size = sectionStart - newFIE->seekpos;/*Überarbeiten*/
                newFIE->next->nr = ++fieIndex;
                newFIE = newFIE->next;
                sectionStart = buf_readline(b, linepointer, LINEBUFFERSIZE);
                newFIE->seekpos = sectionStart;
                newFIE->lines += 1;
            }
            else
            {
                newFIE->lines += 2;
            }
        }
        else
        {
            newFIE->lines += 1;
        }
    }
    return newFI;
}

int main(int argc, char const *argv[])
{
    char *path = "first.txt";
    fi_new(path, "/SECTION");
    return 0;
}
