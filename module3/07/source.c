#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int init_queues(mqd_t *ab, mqd_t *ba)
{
    struct mq_attr attr = {0};
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;

    mq_unlink(QUEUE_AB);
    mq_unlink(QUEUE_BA);

    *ab = mq_open(QUEUE_AB, O_CREAT | O_RDWR, 0644, &attr);
    if (*ab == (mqd_t)-1)
        return -1;

    *ba = mq_open(QUEUE_BA, O_CREAT | O_RDWR, 0644, &attr);
    if (*ba == (mqd_t)-1)
    {
        mq_close(*ab);
        mq_unlink(QUEUE_AB);
        return -1;
    }

    return 0;
}

void close_queues(mqd_t ab, mqd_t ba)
{
    mq_close(ab);
    mq_close(ba);
    mq_unlink(QUEUE_AB);
    mq_unlink(QUEUE_BA);
}

void chat_symmetric(mqd_t send_mq, mqd_t recv_mq)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        char msg[MAX_MSG_SIZE];
        unsigned int prio;

        while (1)
        {
            if (mq_receive(recv_mq, msg, MAX_MSG_SIZE, &prio) >= 0)
            {
                if (prio == TERM_PRIORITY)
                {
                    printf("\n[System] Peer ended chat. Exiting.\n");
                    exit(EXIT_SUCCESS);
                }

                printf("\nPeer: %s\nYou: ", msg);
                fflush(stdout);
            }
            else
            {
                perror("mq_receive");
                exit(EXIT_FAILURE);
            }
        }
    }
    else
    {
        char msg[MAX_MSG_SIZE];
        unsigned int prio;

        printf("Chat started. Type messages. Type 'exit' to quit.\n");

        while (1)
        {
            printf("You: ");
            if (!fgets(msg, MAX_MSG_SIZE, stdin))
                break;

            msg[strcspn(msg, "\n")] = '\0';
            prio = (strcmp(msg, "exit") == 0) ? TERM_PRIORITY : NORMAL_PRIORITY;

            if (mq_send(send_mq, msg, strlen(msg) + 1, prio) == -1)
            {
                perror("mq_send");
                break;
            }

            if (prio == TERM_PRIORITY)
            {
                printf("[System] You ended the chat. Exiting.\n");
                kill(pid, SIGTERM);
                waitpid(pid, NULL, 0);
                break;
            }
        }
    }
}
