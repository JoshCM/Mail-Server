#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "database.h"

int db_update(const char *filepath, const DBRecord *new)
{
	DBRecord newRecord = *new;
	DBRecord oldRecord;
	int fd = open(filepath, O_RDWR);
	int r, index;

	if (fd < 0)
	{
		perror("Fehler beim Öffnen der Datei");
		return -1;
	}
	index = 0;
	while (1)
	{
		r = read(fd, &(oldRecord), sizeof(DBRecord));

		if (r == 0)
		{
			break;
		}

		if (!strcmp(oldRecord.key, newRecord.key) && !strcmp(oldRecord.cat, newRecord.cat))
		{
			lseek(fd, sizeof(DBRecord) * index, SEEK_SET);
			write(fd, new, sizeof(DBRecord));
			return index;
		}

		index++;
	}
	write(fd, new, sizeof(DBRecord));

	return ++index;
}

int db_search(const char *filepath, int start, DBRecord *record)
{
	DBRecord readin;
	int readfd = open(filepath, O_RDONLY);
	int r,idx;

	if (readfd < 0)
	{
		perror("Fehler beim öffnen der Datei");
		return -1;
	}

	lseek(readfd, sizeof(DBRecord) * start, SEEK_SET);
	idx = start;
	while (1)
	{
		r = read(readfd, &(readin), sizeof(DBRecord));

		if (r == 0)
		{
			break;
		}

		if ((!strcmp(record->key, "") && !strcmp(record->cat, "")) ||
			(!strcmp(record->key, readin.key) && !strcmp(record->cat, readin.cat)) ||
			(!strcmp(record->key, readin.key) && !strcmp(record->cat, "")) ||
			(!strcmp(record->cat, readin.cat) && !strcmp(record->key, "")))
		{
			*record = readin;
			close(readfd);
			return idx;
		}

		idx ++;
	}

	return -42;
}

int db_put(const char *filepath, int index, const DBRecord *record)
{
	DBRecord newEntry;
	struct stat *filestat = malloc(sizeof(struct stat));
	int writefd;
	int filesize;

	stat(filepath, filestat);
	filesize = filestat->st_size / sizeof(DBRecord);

	strcpy(newEntry.key, record->key);
	strcpy(newEntry.cat, record->cat);
	strcpy(newEntry.value, record->value);

	if (index < 0 || index > filesize)
	{
		writefd = open(filepath, O_WRONLY | O_APPEND | O_CREAT,0640);
		write(writefd, record, sizeof(DBRecord));
		close(writefd);
	}
	else
	{
		writefd = open(filepath, O_WRONLY);
		lseek(writefd, sizeof(DBRecord) * index, SEEK_SET);
		write(writefd, record, sizeof(DBRecord));
		close(writefd);
	}

	return 0;
}

int db_get(const char *filepath, int index, DBRecord *result)
{
	int readfd = open(filepath, O_RDONLY);
	DBRecord readin;

	if (readfd < 0)
	{
		perror("Probleme beim öffnen der datei\n");
		exit(2);
	}

	lseek(readfd, sizeof(DBRecord) * index, SEEK_SET);
	read(readfd, &(readin), sizeof(DBRecord));

	*result = readin;

	close(readfd);
	return 0;
}

int formatedPrint(DBRecord record, int outfd)
{
	char spaces[] = "                   ";

	write(outfd, record.key, strlen(record.key));
	write(outfd, spaces, strlen(spaces) - strlen(record.key));

	write(outfd, "|", 1);

	write(outfd, record.cat, strlen(record.cat));
	write(outfd, spaces, strlen(spaces) - strlen(record.cat));

	write(outfd, "|", 1);

	write(outfd, record.value, strlen(record.value));
	write(outfd, "\n", 1);

	return 0;
}

int db_del(const char *filepath, int index)
{
	DBRecord *record = malloc(sizeof(DBRecord));
	int fdOld = open(filepath, O_RDWR);
	int fdNew = open("temp.db", O_WRONLY | O_CREAT, 0777);
	int r, idx;

	if (fdOld < 0)
	{
		perror("Fehler beim Öffnen der Datei");
		return -1;
	}
	idx = 0;
	while (1)
	{
		r = read(fdOld, record, sizeof(DBRecord));

		if (r == 0)
		{
			break;
		}

		if (idx == index)
		{
		}
		else
		{
			write(fdNew, record, sizeof(DBRecord));
		}
		idx++;
	}

	rename("temp.db", filepath);

	return 0;
}

int db_list(const char *path, int outfd, int (*filter)(DBRecord *rec, const void *data), const void *data)
{
	int readfd = open(path, O_RDONLY);
	int r;
	DBRecord readin;

	if (readfd < 0)
	{
		perror("Probleme beim öffnen der datei\n");
		exit(2);
	}

	while (1)
	{
		r = read(readfd, &(readin), sizeof(DBRecord));

		if (r == 0)
		{
			break;
		}
		else if (r < 0)
		{
			perror("Lesefehler");
			close(readfd);
			return -1;
		}
		if (filter == NULL)
		{
			formatedPrint(readin, outfd);
		}
		else
		{
			if (filter(&(readin), data))
			{
				formatedPrint(readin, outfd);
			}
		}
	}
	close(readfd);
	return 0;
}

/**int main(int argc, char const *argv[])
{
	DBRecord res = {"","ENTRY",""};
	DBRecord *pres = &res;
	db_list("first.db",1,NULL,NULL);
	db_del("first.db", 2);
	db_list("first.db",1,NULL,NULL);	

	return 0;
}
**/
