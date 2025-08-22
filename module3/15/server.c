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
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

#define BUFF_SIZE 4096
#define MAX_EVENTS 64

int sockfd = -1;
int *nclients;
int epollfd = -1;

void cleanup(int sig)
{
    printf("\nShutting down server...\n");
    munmap(nclients, sizeof(int));
    if (epollfd != -1)
        close(epollfd);
    if (sockfd != -1)
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

// Устанавливаем сокет в неблокирующий режим
int set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

typedef struct {
    int sock;
    char buffer[BUFF_SIZE];
    int buff_len;
    int stage; // 0 - ожидание команды или операции, 1 - ожидание первого числа, 2 - ожидание второго числа, 3 - получение файла
    char op;
    int a, b;
    FILE *fp;
    int file_mode; // 0 - нет, 1 - получение файла
    char filename[256];
} client_t;

client_t clients[FD_SETSIZE]; // Кэш для данных клиентов, индекс по sock

void close_client(int sock)
{
    if (clients[sock].file_mode == 1 && clients[sock].fp)
    {
        fclose(clients[sock].fp);
        clients[sock].fp = NULL;
        printf("File %s received.\n", clients[sock].filename);
    }
    close(sock);
    if (*nclients > 0)
        (*nclients)--;
    clients[sock].sock = -1;
    clients[sock].buff_len = 0;
    clients[sock].stage = 0;
    clients[sock].file_mode = 0;
    printusers();
}


void handle_client_data(int sock)
{
    client_t *cl = &clients[sock];
    int n;

    while (1)
    {
        n = recv(sock, cl->buffer + cl->buff_len, sizeof(cl->buffer) - cl->buff_len - 1, 0);
        if (n == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break; // Все данные прочитаны
            perror("recv");
            close_client(sock);
            return;
        }
        else if (n == 0)
        {
            printf("Client disconnected\n");
            close_client(sock);
            return;
        }
        cl->buff_len += n;
        cl->buffer[cl->buff_len] = 0;

        // Обработка данных по этапам
        if (cl->file_mode == 1)
        {
            // Получаем файл
            if (cl->fp == NULL)
            {
                // Открываем файл для записи
                cl->fp = fopen(cl->filename, "wb");
                if (!cl->fp)
                {
                    perror("File write error");
                    close_client(sock);
                    return;
                }
            }
            fwrite(cl->buffer, 1, cl->buff_len, cl->fp);
            cl->buff_len = 0;

            break;
        }
        else
        {
            // Обработка команд текстом
            if (cl->stage == 0)
            {
                // Ожидаем команду или операцию
                if (cl->buff_len < 1)
                    break;
                if (strncmp(cl->buffer, "exit", 4) == 0)
                {
                    printf("Client disconnected via 'exit'\n");
                    close_client(sock);
                    return;
                }
                else if (strncmp(cl->buffer, "file", 4) == 0)
                {
                    cl->stage = 10; // Ожидание имени файла
                    // Удаляем "file" из буфера
                    memmove(cl->buffer, cl->buffer + 4, cl->buff_len - 4);
                    cl->buff_len -= 4;
                    cl->buffer[cl->buff_len] = 0;
                }
                else
                {
                    // Первая буква — операция
                    cl->op = cl->buffer[0];
                    cl->stage = 1;
                    memmove(cl->buffer, cl->buffer + 1, cl->buff_len - 1);
                    cl->buff_len -= 1;
                    cl->buffer[cl->buff_len] = 0;
                }
            }
            else if (cl->stage == 10)
            {
                char *newline = strchr(cl->buffer, '\n');
                if (!newline)
                    break; // Ждем дальше
                int len = newline - cl->buffer;
                if (len > 255) len = 255;
                strncpy(cl->filename, cl->buffer, len);
                cl->filename[len] = 0;
                // Удаляем имя файла из буфера
                memmove(cl->buffer, newline + 1, cl->buff_len - (len + 1));
                cl->buff_len -= (len + 1);
                cl->buffer[cl->buff_len] = 0;
                cl->file_mode = 1;
                cl->fp = NULL;
                printf("Receiving file: %s\n", cl->filename);
                // Файл начнёт записываться при следующем заходе в recv
            }
            else if (cl->stage == 1)
            {
                // Ожидаем первое число
                char *newline = strchr(cl->buffer, '\n');
                if (!newline)
                    break;
                *newline = 0;
                cl->a = atoi(cl->buffer);
                // Удаляем первое число из буфера
                int rem = cl->buff_len - (newline - cl->buffer + 1);
                memmove(cl->buffer, newline + 1, rem);
                cl->buff_len = rem;
                cl->buffer[cl->buff_len] = 0;
                cl->stage = 2;
            }
            else if (cl->stage == 2)
            {
                // Ожидаем второе число
                char *newline = strchr(cl->buffer, '\n');
                if (!newline)
                    break;
                *newline = 0;
                cl->b = atoi(cl->buffer);
                int result;
                char resp[64];
                if (myfunc(cl->a, cl->b, cl->op, &result))
                    snprintf(resp, sizeof(resp), "%d\n", result);
                else
                    snprintf(resp, sizeof(resp), "Error\n");

                send(sock, resp, strlen(resp), 0);

                // Удаляем число из буфера
                int rem = cl->buff_len - (newline - cl->buffer + 1);
                memmove(cl->buffer, newline + 1, rem);
                cl->buff_len = rem;
                cl->buffer[cl->buff_len] = 0;

                cl->stage = 0; // назад к ожиданию команды
            }
        }
    }
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

    struct sockaddr_in serv_addr;
    socklen_t clilen;
    struct sockaddr_in cli_addr;

    if (argc < 2)
        error("No port provided\n");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    // Сделать серверный сокет неблокирующим
    if (set_nonblocking(sockfd) < 0)
        error("Failed to set nonblocking");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    int portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, 5);

    epollfd = epoll_create1(0);
    if (epollfd == -1)
        error("epoll_create1");

    struct epoll_event ev, events[MAX_EVENTS];

    // Добавляем серверный сокет в epoll
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev) == -1)
        error("epoll_ctl: sockfd");

    // Инициализируем массив клиентов
    for (int i = 0; i < FD_SETSIZE; i++)
    {
        clients[i].sock = -1;
        clients[i].buff_len = 0;
        clients[i].stage = 0;
        clients[i].file_mode = 0;
        clients[i].fp = NULL;
    }

    printf("Server started on port %d.\n", portno);

    while (1)
    {
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            if (errno == EINTR)
                continue;
            error("epoll_wait");
        }

        for (int i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == sockfd)
            {
                // Новый клиент
                while (1)
                {
                    clilen = sizeof(cli_addr);
                    int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
                    if (newsockfd == -1)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break; // Нет больше клиентов
                        else
                            perror("accept");
                    }
                    else
                    {
                        if (set_nonblocking(newsockfd) < 0)
                        {
                            close(newsockfd);
                            continue;
                        }
                        ev.events = EPOLLIN | EPOLLET;
                        ev.data.fd = newsockfd;
                        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, newsockfd, &ev) == -1)
                        {
                            perror("epoll_ctl: newsockfd");
                            close(newsockfd);
                            continue;
                        }
                        // Инициализируем клиента
                        clients[newsockfd].sock = newsockfd;
                        clients[newsockfd].buff_len = 0;
                        clients[newsockfd].stage = 0;
                        clients[newsockfd].file_mode = 0;
                        clients[newsockfd].fp = NULL;

                        (*nclients)++;

                        struct hostent *hst = gethostbyaddr((char *)&cli_addr.sin_addr, sizeof(cli_addr.sin_addr), AF_INET);
                        printf("+%s [%s] new connect!\n", (hst) ? hst->h_name : "Unknown host", inet_ntoa(cli_addr.sin_addr));
                        printusers();
                    }
                }
            }
            else
            {
                // Данные от клиента
                handle_client_data(events[i].data.fd);
            }
        }
    }

    cleanup(0);
    return 0;
}
