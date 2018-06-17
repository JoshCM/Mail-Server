#include "../blatt5/pop3.c"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

#define PORTNUMMER 8080

int main(void)
{
    int sockfd, newsockfd, clientlen, pid;
    struct sockaddr_in servaddr, clientaddr;

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORTNUMMER);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        exit(-1);
    }
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("bind");
        exit(-1);
    }
    if (listen(sockfd, 5) < 0)
    {
        perror("listen");
        exit(-1);
    }

    while (1)
    {
        clientlen = sizeof(struct sockaddr);
        newsockfd = accept(sockfd, (struct sockaddr *)&clientaddr, &clientlen);
        if (newsockfd < 0)
        {
            perror("accept");
            exit(-1);
        }
        pid = fork();
        if (!pid)
        {
            process_pop3(newsockfd, newsockfd);
            close(sockfd);
            break;
        }
        else
        {
            close(newsockfd);
        }
    }
    return 0;
}