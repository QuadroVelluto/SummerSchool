#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    mqd_t mq_ab, mq_ba;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s [ping|pong]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "ping") == 0)
    {
        if (init_queues(&mq_ab, &mq_ba) == -1)
        {
            perror("init_queues");
            return EXIT_FAILURE;
        }
        chat_symmetric(mq_ab, mq_ba);
        close_queues(mq_ab, mq_ba);
    }
    else if (strcmp(argv[1], "pong") == 0)
    {
        mq_ab = mq_open(QUEUE_AB, O_RDWR);
        mq_ba = mq_open(QUEUE_BA, O_RDWR);
        if (mq_ab == (mqd_t)-1 || mq_ba == (mqd_t)-1)
        {
            perror("mq_open");
            return EXIT_FAILURE;
        }
        chat_symmetric(mq_ba, mq_ab);
        mq_close(mq_ab);
        mq_close(mq_ba);
    }
    else
    {
        fprintf(stderr, "Invalid mode '%s', use 'ping' or 'pong'\n", argv[1]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
