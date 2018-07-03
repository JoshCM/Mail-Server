#include "../blatt5/pop3.c"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

#define POP3_PORTNUMMER 8110
#define SMTP_PORTNUMMER 8582

int main(void)
{

    int enable = 1;
    int smtpsock, pop3sock, newsockfd, pid;
    unsigned int clientlen;
    struct sockaddr_in pop3addr, smtpaddr, clientaddr;
    fd_set socketSet;

    FD_ZERO(&socketSet);
    pop3addr.sin_family = AF_INET;
    pop3addr.sin_addr.s_addr = htonl(INADDR_ANY);
    pop3addr.sin_port = htons(POP3_PORTNUMMER);

    pop3addr.sin_family = AF_INET;
    pop3addr.sin_addr.s_addr = htonl(INADDR_ANY);
    pop3addr.sin_port = htons(SMTP_PORTNUMMER);

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

    if (bind(pop3sock, (struct sockaddr *)&pop3addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("bind");
        exit(-1);
    }

    if (bind(smtpsock, (struct sockaddr *)&smtpaddr, sizeof(struct sockaddr_in) < 0))
    {
        perror("bindsmtp");
        exit(-1);
    }
    if (listen(pop3sock, 5) < 0)
    {
        perror("listen");
        exit(-1);
    }

    if (listen(smtpsock,5)<0)
    {
        perror("listensmtp");
        exit(-1);
    }

    
    while (1)
    {
        clientlen = sizeof(struct sockaddr);
        FD_SET(pop3sock,&socketSet);
        FD_SET(smtpsock,&socketSet);
        
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
    return 0;
}
