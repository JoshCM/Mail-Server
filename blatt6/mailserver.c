#include "../blatt5/pop3.c"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../blatt7/smtp.c"

#define POP3_PORTNUMMER 8110
#define SMTP_PORTNUMMER 8582
int zaehler = 0;

void *createThread(void *sock)
{
    int *socket = (int*)sock;
    process_smtp(*socket, *socket);
    return socket;
}

int main(void)
{
    int enable = 1;
    int smtpsock, pop3sock, newsockfd, pid = 0;
    unsigned int clientlen;
    struct sockaddr_in pop3addr, smtpaddr, clientaddr;
    DBRecord portPicker = {"","",""};
    fd_set socketSet;
    pthread_t tid[42];

    FD_ZERO(&socketSet);
    pop3addr.sin_family = AF_INET;
    pop3addr.sin_addr.s_addr = htonl(INADDR_ANY);
    strcpy(portPicker.key, "port");
    strcpy(portPicker.cat, "pop3");
    db_search(DB_PATH,0,&portPicker);
    pop3addr.sin_port = htons(atoi(portPicker.value));

    smtpaddr.sin_family = AF_INET;
    smtpaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    strcpy(portPicker.key, "port");
    strcpy(portPicker.cat, "smtp");
    db_search(DB_PATH,0,&portPicker);
    smtpaddr.sin_port = htons(atoi(portPicker.value));


    if ((pop3sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("pop3socket");
        exit(-1);
    }
    if ((smtpsock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("smtpsocket");
        exit(-1);
    }

    if (setsockopt(pop3sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    if (setsockopt(smtpsock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");


    if (bind(pop3sock, (struct sockaddr *)&pop3addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("bind");
        exit(-1);
    }

    if (bind(smtpsock, (struct sockaddr *)&smtpaddr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("bind");
        exit(-1);
    }

    if (listen(pop3sock, 5) < 0)
    {
        perror("listen");
        exit(-1);
    }

    if (listen(smtpsock, 5) < 0)
    {
        perror("listensmtp");
        exit(-1);
    }

    while (1)
    {
        clientlen = sizeof(struct sockaddr);
        FD_SET(pop3sock, &socketSet);
        FD_SET(smtpsock, &socketSet);
        if (select(5, &socketSet, NULL, NULL, NULL) < 0)
        {
            perror("select");
            exit(-1);
        }

        if (FD_ISSET(pop3sock, &socketSet))
        {

            newsockfd = accept(pop3sock, (struct sockaddr *)&clientaddr, &clientlen);
            if (newsockfd < 0)
            {
                perror("accept");
                exit(-1);
            }
            pid = fork();
            if (!pid)
            {
                process_pop3(newsockfd, newsockfd);
                close(pop3sock);
                break;
            }
            else
            {
                close(newsockfd);
            }
        }

        if (FD_ISSET(smtpsock, &socketSet))
        {
            newsockfd = accept(smtpsock, (struct sockaddr *)&clientaddr, &clientlen);
            if (newsockfd < 0)
            {
                perror("accept");
                exit(-1);
            }
            pthread_create(&tid[zaehler], NULL, createThread, &newsockfd);
            pthread_detach(zaehler);
            zaehler++;
            
        }
    }
    return 0;
}
