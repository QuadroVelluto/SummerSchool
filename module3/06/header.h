#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_TEXT 512
#define MAX_CLIENTS 100

typedef struct
{
    long mtype;
    int client_id;
    char mtext[MAX_TEXT];
} msgbuf;

void run_server(int);
void run_client(int, int);