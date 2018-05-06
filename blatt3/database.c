#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "database.h"

int db_put(const char *filepath, int index,const DBRecord *record){
	DBRecord newEntry;
	int writefd;

	strcpy(newEntry.key,record->key);
	strcpy(newEntry.cat,record->cat);
	strcpy(newEntry.value,record->value);

	if(index == -1){
		writefd = open(filepath,O_WRONLY|O_APPEND);
		write(writefd,record,sizeof(DBRecord));
		close(writefd);
	}

	return 0;
}

int db_get(const char * filepath,int index, DBRecord *result){
	int readfd = open(filepath,O_RDONLY);
	int r,s;
	DBRecord readin;

	if(readfd < 0){
		perror("Probleme beim öffnen der datei\n");
		exit(2);
	}

	s = lseek(readfd,sizeof(DBRecord),SEEK_SET+index-1);
	r = read(readfd,&(readin),sizeof(DBRecord));

	strcpy(result->key,readin.key);
	strcpy(result->cat,readin.cat);
	strcpy(result->value,readin.value);

	close(readfd);
	return 0;
	
}

int formatedPrint(DBRecord record,int outfd){
	char spaces[]= "                   ";

	write(outfd,record.key,strlen(record.key));
	write(outfd,spaces,strlen(spaces)-strlen(record.key));

	write(outfd,"|",1);

	write(outfd,record.cat,strlen(record.cat));
	write(outfd,spaces,strlen(spaces)-strlen(record.cat));

	write(outfd,"|",1);
	
	write(outfd,record.value,strlen(record.value));
	write(outfd,"\n",1);

	return 0;
}

int db_list(const char *path, int outfd, int (*filter)(DBRecord *rec, const void *data),const void *data){
	int readfd = open(path,O_RDONLY);
	int r;
	DBRecord readin;

	if(readfd < 0){
		perror("Probleme beim öffnen der datei\n");
		exit(2);
	}

	while(1){
		r = read(readfd,&(readin), sizeof(DBRecord));

		if(r == 0){
			break;
		}else if(r <0){
			perror("Lesefehler");
			close(readfd);
			return -1;
		}

		formatedPrint(readin,outfd);

	}
		close(readfd);
		return 0;
	}

	



int main(int argc, char const *argv[])
{
	DBRecord res = {"FOURTH","ENTRY","Mein vierter Eintrag."};
	DBRecord *pres = &res;
	db_list("first.db",1,NULL,NULL);
	
	db_get("first.db",0,pres);
	formatedPrint(*pres,1);

	return 0;
}