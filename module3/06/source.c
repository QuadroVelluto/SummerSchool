#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int create_queue(key_t key)
{
    int qid = msgget(key, IPC_CREAT | 0666);
    if (qid == -1)
    {
        perror("msgget/create");
        exit(EXIT_FAILURE);
    }
    return qid;
}

int get_queue(key_t key)
{
    int qid = msgget(key, 0666);
    if (qid == -1)
    {
        perror("msgget/open");
        exit(EXIT_FAILURE);
    }
    return qid;
}

int send_message(int qid, long mtype, int sender, const char *text)
{
    msgbuf msg;
    msg.mtype = mtype;
    msg.sender = sender;
    strncpy(msg.mtext, text, MAX_TEXT - 1);
    msg.mtext[MAX_TEXT - 1] = '\0';
    if (msgsnd(qid, &msg, sizeof(msg) - sizeof(long), 0) == -1)
    {
        perror("msgsnd");
        return -1;
    }
    return 0;
}

int receive_message(int qid, long mtype, msgbuf *msg)
{
    ssize_t ret = msgrcv(qid, msg, sizeof(*msg) - sizeof(long), mtype, 0);
    if (ret == -1)
    {
        if (errno != ENOMSG)
            perror("msgrcv");
        return -1;
    }
    return 0;
}