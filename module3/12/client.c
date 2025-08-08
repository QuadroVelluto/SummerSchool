#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

int main(int argc, char **argv)
{
    int sockfd;
    int n;
    char sendline[1000], recvline[1000];
    struct sockaddr_in servaddr, cliaddr;

    if (argc != 2)
    {
        printf("Usage: %s <IP address>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(0);
    cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0)
    {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(51000);
    if (inet_aton(argv[1], &servaddr.sin_addr) == 0)
    {
        fprintf(stderr, "Invalid IP address\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char name[64] = {};
    printf("Enter your name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';

    // Отправляем серверу сообщение о присоединении
    snprintf(sendline, sizeof(sendline), "join:%s", name);
    if (sendto(sockfd, sendline, strlen(sendline) + 1, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("sendto");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) // Дочериний процесс получает сообщения
    {
        socklen_t len = sizeof(servaddr);
        while (1)
        {
            n = recvfrom(sockfd, recvline, sizeof(recvline) - 1, 0, (struct sockaddr *)&servaddr, &len);
            if (n < 0)
            {
                perror("recvfrom");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            recvline[n] = '\0';
            printf("%s\n", recvline);
        }
    }
    else // Родительский процесс отправляет сообщения
    {
        while (1)
        {
            char line[935] = {};
            fgets(line, sizeof(line), stdin);
            line[strcspn(line, "\n")] = '\0';

            if (strcmp(line, "exit") == 0)
            {
                snprintf(sendline, sizeof(sendline), "exit:%s", name);
                sendto(sockfd, sendline, strlen(sendline) + 1, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

                kill(pid, SIGTERM); // Завершаем приём
                wait(NULL);         // Ждём завершения дочернего
                break;
            }

            snprintf(sendline, sizeof(sendline), "%s > %s", name, line);

            if (sendto(sockfd, sendline, strlen(sendline) + 1, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
            {
                perror("sendto");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
        }
    }

    close(sockfd);
    return 0;
}
