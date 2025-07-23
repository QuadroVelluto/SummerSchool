#define MIN_PRIORITY 256
#define MAX_MESSAGE_LEN 256

typedef struct Node
{
    char message[MAX_MESSAGE_LEN];
    struct Node *next;
} Node;

typedef struct
{
    Node *head[MIN_PRIORITY];
    Node *tail[MIN_PRIORITY];
} PriorityQueue;

void pq_init(PriorityQueue *q);
void pq_enqueue(PriorityQueue *q, int priority, const char *message);
char *pq_dequeue(PriorityQueue *q);
char *pq_dequeue_priority(PriorityQueue *q, int priority);
char *pq_dequeue_min_priority(PriorityQueue *q, int min_priority);
void pq_print_all(const PriorityQueue *q);
void pq_free(PriorityQueue *q);
void menu();