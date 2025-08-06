#include <mqueue.h>

#define QUEUE_AB "/mq_chat_ab"
#define QUEUE_BA "/mq_chat_ba"
#define MAX_MSG_SIZE 256
#define MAX_MESSAGES 10
#define TERM_PRIORITY 10
#define NORMAL_PRIORITY 1

int init_queues(mqd_t *ab, mqd_t *ba);
void close_queues(mqd_t ab, mqd_t ba);
void chat_symmetric(mqd_t send_mq, mqd_t recv_mq);
