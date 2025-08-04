#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_TEXT 512
#define SERVER_TYPE 10


typedef struct
{
    long mtype;
    int sender;
    char mtext[MAX_TEXT];
} msgbuf;

int create_queue(key_t key);
int get_queue(key_t key);

int send_message(int qid, long mtype, int sender, const char *text);
int receive_message(int qid, long mtype, msgbuf *msg);
