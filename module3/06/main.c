#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <stdbool.h>

#define FTOK_PATH "/tmp"
#define FTOK_PROJID 'C'
#define MAX_CLIENTS 100

static int qid;
static bool clients[MAX_CLIENTS + 1] = {false};

void cleanup(int sig)
{
    printf("\nServer: removing queue %d\n", qid);
    msgctl(qid, IPC_RMID, NULL);
    exit(EXIT_SUCCESS);
}

void run_server(void)
{
    msgbuf msg;
    signal(SIGINT, cleanup);
    printf("Server started. Queue ID = %d\n", qid);
    while (true)
    {
        if (receive_message(qid, SERVER_TYPE, &msg) < 0)
            continue;

        int sender = msg.sender;
        int cid = sender / 10;

        printf("Received from %d: %s\n", sender, msg.mtext);

        if (strcmp(msg.mtext, "shutdown") == 0)
        {
            if (cid >= 2 && cid <= MAX_CLIENTS)
            {
                clients[cid] = false;
                printf("Client %d disconnected.\n", cid);
            }
            continue;
        }

        if (cid >= 2 && cid <= MAX_CLIENTS)
            clients[cid] = true;

        for (int id = 2; id <= MAX_CLIENTS; ++id)
            if (clients[id] && id * 10 != sender)
                send_message(qid, id * 10, sender, msg.mtext);
    }
}

void run_client(int client_id)
{
    pid_t pid;
    int my_type = client_id * 10;
    char line[MAX_TEXT];

    pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        msgbuf msg;
        while (true)
            if (receive_message(qid, my_type, &msg) == 0)
                printf("[from %d] %s\n", msg.sender, msg.mtext);
        exit(EXIT_SUCCESS);
    }


    printf("Client %d connected. Queue ID = %d\n", client_id, qid);
    while (fgets(line, sizeof(line), stdin))
    {
        line[strcspn(line, "\n")] = '\0';

        if (send_message(qid, SERVER_TYPE, my_type, line) < 0)
            break;

        if (strcmp(line, "shutdown") == 0)
        {
            kill(pid, SIGTERM);
            waitpid(pid, NULL, 0);
            exit(EXIT_SUCCESS);
        }
    }
}

int main(int argc, char **argv)
{
    key_t key = ftok(FTOK_PATH, FTOK_PROJID);
    if (key == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    if (argc == 2 && strcmp(argv[1], "server") == 0)
    {
        qid = create_queue(key);
        run_server();
    }
    else if (argc == 2)
    {
        int cid = atoi(argv[1]);

        if (cid < 2 || cid > MAX_CLIENTS)
        {
            fprintf(stderr, "Usage: %s server | <client_id 2..%d>\n", argv[0], MAX_CLIENTS);
            exit(EXIT_FAILURE);
        }

        qid = get_queue(key);
        // signal(SIGINT, SIG_DFL);
        run_client(cid);
    }
    else
    {
        fprintf(stderr, "Usage: %s server | <client_id>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    return 0;
}