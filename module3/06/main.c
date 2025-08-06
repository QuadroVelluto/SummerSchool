#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/msg.h>
#include <stdio.h>
#include "header.h"

#define PATHNAME "/tmp"
#define PROJ_ID 'A'

int main(int argc, char *argv[])
{
    // Подсказка для использования
    if (argc != 2)
    {
        printf("Usage: %s <\"server\"/client_id = (1..%d)>\n", argv[0], MAX_CLIENTS);
        exit(EXIT_SUCCESS);
    }
    // Создание ключа IPC
    key_t key = ftok(PATHNAME, PROJ_ID);
    if (key == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "server") == 0) // Запуск сервера
    {
        // Создание очереди сообщений System V
        int mq = msgget(key, IPC_CREAT | 0666);
        if (mq == -1)
        {
            perror("msgget");
            exit(EXIT_FAILURE);
        }

        run_server(mq);
    }
    else // Запуск клиента
    {
        // Подключение к очереди сообщений System V
        int mq = msgget(key, 0666);
        if (mq == -1)
        {
            perror("msgget");
            exit(EXIT_FAILURE);
        }

        int cid = atoi(argv[1]);// ID клиента - 2, 3, ..., MAX_CLIENTS
        if (cid < 1 || cid > MAX_CLIENTS)
        {
            fprintf(stderr, "Invalid client_id. Must be between 1 and %d\n", MAX_CLIENTS);
            exit(EXIT_FAILURE);
        }

        run_client(mq, cid);
    }

    exit(EXIT_SUCCESS);
}