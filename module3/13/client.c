#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int my_sock, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buff[4096];

    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0)
        error("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(my_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    while (1)
    {
        printf("Enter operation (+, -, *, /, file) or 'exit': ");
        fgets(buff, sizeof(buff), stdin);
        buff[strcspn(buff, "\n")] = '\0';

        if (strcmp(buff, "+") != 0 &&
            strcmp(buff, "-") != 0 &&
            strcmp(buff, "*") != 0 &&
            strcmp(buff, "/") != 0 &&
            strncmp(buff, "file", 4) != 0 &&
            strncmp(buff, "exit", 4) != 0
        )
        {
            printf("Invalid operation. Please enter one of: +, -, *, /, file, exit.\n");
            continue;
        }
        send(my_sock, buff, strlen(buff), 0);
        
        if (!strncmp(buff, "exit", 4))
        {
            printf("Exit...\n");
            break;
        }

        if (!strncmp(buff, "file", 4))
        {
            char filename[256];
            printf("Enter filename to send: ");
            fgets(filename, sizeof(filename), stdin);
            filename[strcspn(filename, "\n")] = 0;

            FILE *fp = fopen(filename, "rb");
            if (!fp)
            {
                perror("File open error");
                continue;
            }
            
            send(my_sock, filename, strlen(filename), 0);
            sleep(1);

            while ((n = fread(buff, 1, sizeof(buff), fp)) > 0)
                send(my_sock, buff, n, 0);

            fclose(fp);
            printf("File sent.\n");
            continue;
        }

        printf("Enter first number: ");
        fgets(buff, sizeof(buff), stdin);
        send(my_sock, buff, strlen(buff), 0);

        printf("Enter second number: ");
        fgets(buff, sizeof(buff), stdin);
        send(my_sock, buff, strlen(buff), 0);

        n = recv(my_sock, buff, sizeof(buff) - 1, 0);
        if (n <= 0)
            break;
        buff[n] = 0;
        printf("Result: %s\n", buff);
    }

    close(my_sock);
    return 0;
}