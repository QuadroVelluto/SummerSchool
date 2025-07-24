#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "header.h"

void pq_init(PriorityQueue *q)
{
    for (int i = 0; i < MIN_PRIORITY; ++i)
        q->head[i] = q->tail[i] = NULL;
}

void pq_enqueue(PriorityQueue *q, int priority, const char *message)
{
    if (priority < 0 || priority >= MIN_PRIORITY)
        return;

    Node *new_node = (Node *)malloc(sizeof(Node));
    strncpy(new_node->message, message, MAX_MESSAGE_LEN);
    new_node->message[MAX_MESSAGE_LEN - 1] = '\0';
    new_node->next = NULL;

    if (q->tail[priority])
        q->tail[priority]->next = new_node;
    else
        q->head[priority] = new_node;

    q->tail[priority] = new_node;
}

char *pq_dequeue(PriorityQueue *q)
{
    for (int i = 0; i < MIN_PRIORITY; ++i)
        if (q->head[i])
            return pq_dequeue_priority(q, i);

    return NULL;
}

char *pq_dequeue_priority(PriorityQueue *q, int priority)
{
    if (priority < 0 || priority >= MIN_PRIORITY || !q->head[priority])
        return NULL;

    Node *tmp = q->head[priority];
    char *result = strdup(tmp->message);
    q->head[priority] = tmp->next;
    if (!q->head[priority])
        q->tail[priority] = NULL;

    free(tmp);
    return result;
}

char *pq_dequeue_min_priority(PriorityQueue *q, int min_priority)
{
    if (min_priority < 0 || min_priority >= MIN_PRIORITY - 1)
        return NULL;

    for (int i = min_priority - 1; i >= 0; --i)
        if (q->head[i])
            return pq_dequeue_priority(q, i);

    return NULL;
}


void pq_print_all(const PriorityQueue *q)
{
    printf("Очередь сообщений:\n");
    for (int i = 0; i < MIN_PRIORITY; ++i)
    {
        Node *cur = q->head[i];
        while (cur)
        {
            printf("[Приоритет %d] %s\n", i, cur->message);
            cur = cur->next;
        }
    }
}

void pq_free(PriorityQueue *q)
{
    for (int i = 0; i < MIN_PRIORITY; ++i)
    {
        Node *cur = q->head[i];
        while (cur)
        {
            Node *tmp = cur;
            cur = cur->next;
            free(tmp);
        }
        q->head[i] = q->tail[i] = NULL;
    }
}

void menu()
{
    srand((unsigned int)time(NULL));
    PriorityQueue queue;
    pq_init(&queue);

    int choice;
    char message[256];
    int priority;

    do
    {
        printf("\nМЕНЮ\n");
        printf("[1] Добавить сообщение\n");
        printf("[2] Извлечь сообщение с наивысшим приоритетом\n");
        printf("[3] Извлечь сообщение с заданным приоритетом\n");
        printf("[4] Извлечь сообщение с приоритетом не выше заданного\n");
        printf("[5] Показать все сообщения\n");
        printf("[6] Сгенерировать случайные сообщения\n");
        printf("[0] Выход\n");
        printf("Выберите пункт: ");
        scanf("%d", &choice);
        getchar();

        switch (choice)
        {
        case 1:
            printf("Введите сообщение: ");
            fgets(message, sizeof(message), stdin);
            message[strcspn(message, "\n")] = 0;

            printf("Введите приоритет (0 - %d): ", MIN_PRIORITY);
            scanf("%d", &priority);
            getchar();

            pq_enqueue(&queue, priority, message);
            printf("Сообщение добавлено.\n");
            break;

        case 2:
        {
            char *msg = pq_dequeue(&queue);
            if (msg)
            {
                printf("Извлечено: %s\n", msg);
                free(msg);
            }
            else
            {
                printf("Очередь пуста.\n");
            }
            break;
        }

        case 3:
            printf("Введите приоритет (0 - %d): ", MIN_PRIORITY);
            scanf("%d", &priority);
            getchar();
            {
                char *msg = pq_dequeue_priority(&queue, priority);
                if (msg)
                {
                    printf("Извлечено: %s\n", msg);
                    free(msg);
                }
                else
                {
                    printf("Нет сообщений с таким приоритетом.\n");
                }
            }
            break;

        case 4:
            printf("Введите максимальный приоритет (0 - %d): ", MIN_PRIORITY);
            scanf("%d", &priority);
            getchar();
            {
                char *msg = pq_dequeue_min_priority(&queue, priority);
                if (msg)
                {
                    printf("Извлечено: %s\n", msg);
                    free(msg);
                }
                else
                {
                    printf("Нет сообщений с приоритетом не выше %d.\n", priority);
                }
            }
            break;

        case 5:
            pq_print_all(&queue);
            break;
        case 6:
        {
            int count;
            printf("Сколько сообщений сгенерировать? ");
            scanf("%d", &count);
            getchar();
            for (int i = 0; i < count; ++i)
            {
                int p = rand() % MIN_PRIORITY;
                snprintf(message, sizeof(message), "Сообщение #%d", i + 1);
                pq_enqueue(&queue, p, message);
            }
            printf("Сгенерировано %d сообщений.\n", count);
            break;
        }

        case 0:
            break;

        default:
            printf("Неверный пункт меню.\n");
            break;
        }

    } while (choice != 0);

    pq_free(&queue);
}
