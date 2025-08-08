#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/mman.h>

int sockfd = -1;
int *nclients;

#define BUFF_SIZE 4096

void cleanup(int sig)
{
    printf("\nShutting down server...\n");
    munmap(nclients, sizeof(int));
    close(sockfd);
    exit(0);
}

void printusers()
{
    printf(*nclients ? "%d user(s) online\n" : "No users online\n", *nclients);
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int myfunc(int a, int b, char op, int *result)
{
    switch (op)
    {
    case '+':
        *result = a + b;
        return 1;
    case '-':
        *result = a - b;
        return 1;
    case '*':
        *result = a * b;
        return 1;
    case '/':
        if (b == 0)
            return 0;
        *result = a / b;
        return 1;
    default:
        return 0;
    }
}

void dostuff(int sock)
{
    char buff[BUFF_SIZE], op;
    int a, b, result;
    int n;

    while (1)
    {

        bzero(buff, sizeof(buff));
        n = recv(sock, buff, sizeof(buff) - 1, 0);
        if (n <= 0)
            break;
        buff[n] = 0;

        if (strncmp(buff, "exit", 4) == 0)
        {
            printf("Client disconnected via 'exit'\n");
            break;
        }

        if (strncmp(buff, "file", 4) == 0)
        {

            char filename[256];
            bzero(filename, sizeof(filename));
            n = recv(sock, filename, sizeof(filename) - 1, 0);
            if (n <= 0)
                break;
            filename[n] = 0;

            FILE *fp = fopen(filename, "wb");
            if (!fp)
            {
                perror("File write error");
                continue;
            }

            while ((n = recv(sock, buff, sizeof(buff), 0)) > 0)
            {
                fwrite(buff, 1, n, fp);
                if (n < sizeof(buff))
                    break;
            }
            fclose(fp);
            printf("File %s received.\n", filename);
        }
        else
        {

            op = buff[0];

            bzero(buff, sizeof(buff));
            if ((n = recv(sock, buff, sizeof(buff) - 1, 0)) <= 0)
                break;
            buff[n] = 0;
            a = atoi(buff);

            bzero(buff, sizeof(buff));
            if ((n = recv(sock, buff, sizeof(buff) - 1, 0)) <= 0)
                break;
            buff[n] = 0;
            b = atoi(buff);

            if (myfunc(a, b, op, &result))
                snprintf(buff, sizeof(buff), "%d\n", result);
            else
                snprintf(buff, sizeof(buff), "Error\n");

            send(sock, buff, strlen(buff), 0);
        }
    }

    close(sock);
    (*nclients)--;
    printusers();
}

int main(int argc, char *argv[])
{
    signal(SIGINT, cleanup);

    nclients = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (nclients == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }
    *nclients = 0;

    int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2)
        error("No port provided\n");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    printf("Server started.\n");
    while (1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        (*nclients)++;

        struct hostent *hst;
        hst = gethostbyaddr((char *)&cli_addr.sin_addr, 4, AF_INET);
        printf("+%s [%s] new connect!\n", (hst) ? hst->h_name : "Unknown host", inet_ntoa(cli_addr.sin_addr));
        printusers();

        pid = fork();
        if (pid < 0)
            error("ERROR on fork");

        if (pid == 0)
        {
            close(sockfd);
            dostuff(newsockfd);
            exit(0);
        }
        else
            close(newsockfd);
    }

    close(sockfd);
    return 0;
}
