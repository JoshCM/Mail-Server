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
    int fd = open(filepath, O_RDONLY | O_CREAT, 0640);
    int sectionEnd, lineStart = 0;
    int fieIndex = 1;
    FileIndex *newFI = calloc(1, sizeof(FileIndex));
    FileIndexEntry *newFIE = calloc(1, sizeof(FileIndexEntry));
    LineBuffer *b = buf_new(fd, "\n");
    char line[LINEBUFFERSIZE];
    char *linepointer = line;
    int templines = 0;
    if (fd == -1){
        perror("Wars nix mit oeffnen");
        return NULL;
    }
    newFI->filepath = filepath;
    newFI->entries = newFIE;
    newFI->nEntries++;

    newFIE->nr = fieIndex;

    while (1)
    {
        lineStart = buf_readline(b, linepointer, LINEBUFFERSIZE);
        if (lineStart < 0)
        {
            newFIE->size = sectionEnd - newFIE->seekpos;
            newFI->totalSize += newFIE->size;
            break;
        }

        if (!strncmp(linepointer, separator, strlen(separator)))
        {
            lineStart = buf_readline(b, linepointer, LINEBUFFERSIZE);
            newFIE->seekpos = lineStart;
            newFIE->lines += 1;
            sectionEnd = buf_where(b);
        }
        else if (!strcmp(linepointer, ""))
        {
            templines++;
            lineStart = buf_readline(b, linepointer, LINEBUFFERSIZE);
            if (lineStart < 0)
            {
                continue;
            }

            while (1)
            {
                if (!strncmp(linepointer, separator, strlen(separator)))
                {
                    newFIE->next = calloc(1, sizeof(FileIndexEntry));
                    newFI->nEntries++;
                    newFIE->size = sectionEnd - newFIE->seekpos;
                    newFI->totalSize += newFIE->size;
                    newFIE->next->nr = ++fieIndex;
                    newFIE = newFIE->next;
                    lineStart = buf_readline(b, linepointer, LINEBUFFERSIZE);
                    newFIE->seekpos = lineStart;
                    newFIE->lines += 1;
                    sectionEnd = buf_where(b);
                    templines = 0;
                    break;
                }
                else if (!strcmp(linepointer, ""))
                {
                    templines++;
                    lineStart = buf_readline(b, linepointer, LINEBUFFERSIZE);
                }
                else
                {
                    templines++;
                    newFIE->lines += templines;
                    sectionEnd = buf_where(b);
                    templines = 0;
                    break;
                }
            }
        }
        else
        {
            newFIE->lines += 1;
            sectionEnd = buf_where(b);
        }
    }
    buf_dispose(b);
    close(fd);
    return newFI;
}

void fi_dispose(FileIndex *fi)
{
    FileIndexEntry *cfi = fi->entries;
    FileIndexEntry *pfi = NULL;

    while (cfi != NULL)
    {
        pfi = cfi;
        cfi = cfi->next;
        free(pfi);
    }

    free(fi);
}

FileIndexEntry *fi_find(FileIndex *fi, int n)
{
    FileIndexEntry *found = fi->entries;
    while (--n)
    {
        if (found->next)
        {
            found = found->next;
        }
        else
        {
            return NULL;
        }
    }
    return found;
}

int fi_compactify(FileIndex *fi)
{
    const char *oldfilename = fi->filepath;
    const char *tempfilename = "tmp.txt";
    FileIndexEntry *fie = fi->entries;
    LineBuffer *b = buf_new(open(fi->filepath, O_RDONLY), "\n");
    int newfd = open(tempfilename, O_WRONLY | O_CREAT | O_TRUNC, 0640);
    int linemax = 1024;
    char *line = calloc(sizeof(char) * linemax,1);
    char *seperator = calloc(sizeof(char) * linemax,1);
    FileIndex *newFI;

    buf_readline(b, seperator, linemax);

    while (fie)
    {
        if (fie->del_flag)
        {
            fie = fie->next;
            continue;
        }
        write(newfd, seperator, strlen(seperator));
        write(newfd, b->linesep, b->lineseplen);
        buf_seek(b, fie->seekpos);

        for (; fie->lines > 0; fie->lines--)
        {
            buf_readline(b, line, linemax);
            write(newfd, line, strlen(line));
            write(newfd, b->linesep, b->lineseplen);
        }

        write(newfd, b->linesep, b->lineseplen);
        fie = fie->next;
    }
    free(line);
    free(seperator);
    fi_dispose(fi);
    rename(tempfilename, oldfilename);
    newFI = fi_new(oldfilename, seperator);
    *fi = *newFI;
    return 0;
}

/**
int main(int argc, char const *argv[])
{
    char *path = "joendhard.mbox";
    FileIndex *newFI;
    FileIndexEntry *foundFIE;
    LineBuffer *b = NULL;
    char *line = calloc(1024,1);
    int lines;

    b = buf_new(open(path, O_RDONLY), "\n");
    newFI = fi_new(path, "From ");
    foundFIE = newFI->entries;
    buf_seek(b,foundFIE->seekpos);
    for (lines = 1; lines <= foundFIE->lines; lines++)
    {
        buf_readline(b, line, 1024);
        printf("%d: %s\n", lines, line);
    }

    return 0;
}
**/
