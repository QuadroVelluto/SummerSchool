#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>

int mq_global;

void cleanup_and_exit(int signum)
{
    printf("\nShutting down server (signal %d)...\n", signum);
    if (msgctl(mq_global, IPC_RMID, NULL) == -1)
        perror("msgctl(IPC_RMID)");
    exit(EXIT_SUCCESS);
}

void run_server(int mq)
{
    mq_global = mq;
    signal(SIGINT, cleanup_and_exit); // обработка Ctrl+C

    printf("Server started, mq = %d\n", mq);

    bool client_connected[MAX_CLIENTS + 1] = {false};

    msgbuf buffer;
    while (1)
    {
        if (msgrcv(mq, &buffer, sizeof(buffer) - sizeof(long), 1, 0) == -1)
        {
            if (errno == EINTR)
                continue; // сигнал, перезапустить msgrcv
            perror("msgrcv");
            break;
        }

        int cid = buffer.client_id;

        if (!client_connected[cid])
        {
            client_connected[cid] = true;
            printf("Client(%d) connected\n", cid);
        }

        printf("Client(%d): %s\n", cid, buffer.mtext);

        if (strcmp(buffer.mtext, "shutdown") == 0)
        {
            client_connected[cid] = false;
            char line[MAX_TEXT];
            snprintf(line, sizeof(line), "Client(%d) disconnected", cid);
            printf("%s\n", line);
            strncpy(buffer.mtext, line, MAX_TEXT);
        }

        for (int target = 1; target <= MAX_CLIENTS; target++)
        {
            if (target == cid || !client_connected[target])
                continue;

            buffer.mtype = target;
            if (msgsnd(mq, &buffer, sizeof(buffer) - sizeof(long), 0) == -1)
            {
                perror("msgsnd");
                break;
            }
        }
    }

    cleanup_and_exit(0);
}

void run_client(int mq, int client_id)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        msgbuf buffer;
        while (1)
        {
            if (msgrcv(mq, &buffer, sizeof(buffer) - sizeof(long), client_id, 0) == -1)
            {
                if (errno == EINTR)
                    continue;
                perror("msgrcv (client)");
                exit(EXIT_FAILURE);
            }
            printf("Client(%d)> %s\n", buffer.client_id, buffer.mtext);
        }
    }

    msgbuf buffer;
    while (1)
    {
        printf("> ");
        if (fgets(buffer.mtext, MAX_TEXT, stdin) == NULL)
            break;

        buffer.mtext[strcspn(buffer.mtext, "\n")] = '\0'; // remove newline
        buffer.mtype = 1;
        buffer.client_id = client_id;

        if (msgsnd(mq, &buffer, sizeof(buffer) - sizeof(long), 0) == -1)
        {
            perror("msgsnd");
            break;
        }

        if (strcmp(buffer.mtext, "shutdown") == 0)
        {
            kill(pid, SIGTERM); // убиваем дочерний процесс
            wait(NULL);         // ожидаем его завершения
            break;
        }
    }

    printf("Client exiting...\n");
    exit(EXIT_SUCCESS);
}
