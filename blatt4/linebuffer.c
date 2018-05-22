#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "linebuffer.h"

LineBuffer *buf_new(int descriptor, const char *linesep)
{
    LineBuffer *buffer = calloc(1, sizeof(LineBuffer));
    buffer->descriptor = descriptor;
    buffer->linesep = linesep;
    buffer->lineseplen = strlen(linesep);

    return buffer;
}

void buf_dispose(LineBuffer *b)
{
    free(b);
}

void pump(LineBuffer *b)
{
    b->here = 0;
    b->end = read(b->descriptor, b->buffer, LINEBUFFERSIZE);
    b->bytesread += b->end;
}

int buf_readline(LineBuffer *b, char *line, int linemax)
{
    char lineString[linemax];
    char *sep;
    int index = 0;

    lineString[linemax] = 0;

    while (index < linemax)
    {
        if (b->end == b->here)
        {
            pump(b);
            if (b->end == 0)
            {
                lineString[index] = 0;
                break;
            }
        }

        lineString[index] = b->buffer[b->here++];
        sep = strstr(lineString, b->linesep);

        if (sep)
        {
            *sep = 0;
            break;
        }

        index++;
    }
    strcpy(line, lineString);
    return 0;
}

int buf_where(LineBuffer*b)
{
    return b->bytesread;
}

int main(int argc, char const *argv[])
{
    const char *sep = "\n";
    char *path = "first.txt";
    int linemax = 1024;
    char *line = malloc(sizeof(char) * linemax);
    int fd = open(path, O_RDONLY);
    LineBuffer *lb = buf_new(fd, sep);
    buf_readline(lb, line, 1024);

    while (*line)
    {
        printf("%s\n", line);
        buf_readline(lb, line, 1024);
    }

    return 0;
}
