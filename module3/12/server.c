#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

int sockfd;

#define MAX_CLIENTS 100

int is_new_client(struct sockaddr_in *cliaddr, struct sockaddr_in *clients, int client_count)
{
    for (int i = 0; i < client_count; i++)
        if (clients[i].sin_addr.s_addr == cliaddr->sin_addr.s_addr && clients[i].sin_port == cliaddr->sin_port)
            return 0;

    return 1;
}

void remove_client(struct sockaddr_in *cliaddr, struct sockaddr_in *clients, int *client_count)
{
    for (int i = 0; i < *client_count; i++)
    {
        if (clients[i].sin_addr.s_addr == cliaddr->sin_addr.s_addr && clients[i].sin_port == cliaddr->sin_port)
        {
            // Заменим на последнего клиента
            clients[i] = clients[--(*client_count)];
            break;
        }
    }
}

void handle_sigint(int sig)
{
    printf("\nShutting down server...\n");
    close(sockfd);
    exit(EXIT_SUCCESS);
}

int main()
{
    signal(SIGINT, handle_sigint);

    socklen_t clilen;
    int n;
    char line[1000];
    struct sockaddr_in servaddr, cliaddr;
    struct sockaddr_in clients[MAX_CLIENTS];
    int client_count = 0;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(51000);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Server started\n");

    while (1)
    {
        clilen = sizeof(cliaddr);
        n = recvfrom(sockfd, line, sizeof(line) - 1, 0, (struct sockaddr *)&cliaddr, &clilen);
        if (n < 0)
        {
            perror("recvfrom");
            close(sockfd);
            exit(1);
        }

        line[n] = '\0';

        if (is_new_client(&cliaddr, clients, client_count) && client_count < MAX_CLIENTS)
        {
            clients[client_count++] = cliaddr;
        }

        if (strncmp(line, "join:", 5) == 0)
        {
            char *client_name = line + 5;
            printf("Client '%s' joined the chat.\n", client_name);
            snprintf(line, sizeof(line), "%s joined the chat", client_name);
        }
        else if (strncmp(line, "exit:", 5) == 0)
        {
            char *client_name = line + 5;
            printf("Client '%s' left the chat.\n", client_name);
            snprintf(line, sizeof(line), "%s left the chat", client_name);

            remove_client(&cliaddr, clients, &client_count);
        }
        else
        {
            printf("%s\n", line);
        }

        for (int i = 0; i < client_count; i++)
        {
            if (clients[i].sin_addr.s_addr == cliaddr.sin_addr.s_addr && clients[i].sin_port == cliaddr.sin_port)
                continue;

            if (sendto(sockfd, line, strlen(line) + 1, 0, (struct sockaddr *)&clients[i], sizeof(clients[i])) < 0)
                perror("sendto");
        }
    }

    return 0;
}
