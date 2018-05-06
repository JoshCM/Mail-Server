#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "database.h"

int db_put(const char *filepath, int index,const DBRecord *record){
	return 0;
}

int db_list(const char *path, int outfd, int (*filter)(DBRecord *rec, const void *data),const void *data){
	int readfd = open(path,O_WRONLY);
	DBRecord readin;

	if(readfd < 0){
		perror("Probleme beim öffnen der datei\n");
		exit(2);
	}

	if(filter == NULL){
		while(1){
			read(readfd,&readin,sizeof(DBRecord));
			write(1,&(readin.key),strlen(readin.key));
			write(1,&(readin.cat),strlen(readin.cat));
			write(1,&(readin.value),strlen(readin.value));
			write(1,"\n",1);
			close(readfd);
		}
	}

	return 0;
}


int main(int argc, char const *argv[])
{
	db_list("first.db",1,NULL,NULL);
	return 0;
}