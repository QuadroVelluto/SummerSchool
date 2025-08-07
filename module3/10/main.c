#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>

#define MAX_COUNT 5
#define MIN_VAL 0
#define MAX_VAL 10

struct SharedData
{
    int numbers[MAX_COUNT];
    int count;
    int min;
    int max;
    int ready; // 0 - родитель записал, 1 - ребенок обработал
};

int shmid;
struct SharedData *data;
int sets_processed = 0;
pid_t child_pid;

void cleanup(int signum)
{
    printf("\rClosing program...\n");

    // Убиваем дочерний процесс
    if (child_pid > 0)
    {
        kill(child_pid, SIGTERM);
        waitpid(child_pid, NULL, 0); // Ждём завершения
    }

    // Очистка разделяемой памяти
    shmdt(data);
    shmctl(shmid, IPC_RMID, NULL);

    printf("Number of data sets processed: %d\n", sets_processed);
    exit(0);
}

int main()
{
    signal(SIGINT, cleanup); // Только родитель обрабатывает сигнал
    srand(time(NULL));

    key_t key = ftok("shmfile", 65);
    shmid = shmget(key, sizeof(struct SharedData), 0666 | IPC_CREAT);
    data = (struct SharedData *)shmat(shmid, NULL, 0);

    child_pid = fork();

    if (child_pid < 0)
    {
        perror("fork");
        return 1;
    }
    else if (child_pid == 0)
    {
        // Дочерний процесс игнорирует SIGINT
        signal(SIGINT, SIG_IGN);

        while (1)
        {
            if (data->ready == 0)
            {
                int min = data->numbers[0];
                int max = data->numbers[0];
                for (int i = 1; i < data->count; i++)
                {
                    if (data->numbers[i] < min)
                        min = data->numbers[i];
                    if (data->numbers[i] > max)
                        max = data->numbers[i];
                }
                data->min = min;
                data->max = max;
                data->ready = 1;
            }
            sleep(1);
        }
    }
    else
    {
        // Родительский процесс
        while (1)
        {
            data->count = rand() % ( MAX_COUNT - 1) + 2;
            for (int i = 0; i < data->count; i++)
                data->numbers[i] = rand() % (MAX_VAL - MIN_VAL + 1) + MIN_VAL;

            data->ready = 0;

            // Ждёv пока ребенок обработает
            while (data->ready != 1)
                sleep(1);

            printf("Set %d: ", sets_processed + 1);
            for (int i = 0; i < data->count; i++)
                printf("%d ", data->numbers[i]);

            printf("\n\tMin: %d, Max: %d\n", data->min, data->max);

            sets_processed++;
            sleep(1); // задержка для наглядности
        }
    }

    return 0;
}
