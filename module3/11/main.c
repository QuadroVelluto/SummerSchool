#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>

#define MAX_COUNT 5
#define MIN_VAL 0
#define MAX_VAL 10
#define PATH "/shmfile"

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
void *ptr;

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
    if (munmap(ptr, sizeof(struct SharedData)) == -1)
    {
        perror("munmap");
        exit(1);
    }

    if (close(shmid) == -1)
    {
        perror("close");
        exit(1);
    }

    if (shm_unlink(PATH) == -1)
    {
        perror("shm_unlink");
        exit(1);
    }

    printf("Number of data sets processed: %d\n", sets_processed);
    exit(EXIT_SUCCESS);
}

int main()
{
    signal(SIGINT, cleanup); // Только родитель обрабатывает сигнал
    srand(time(NULL));

    int shmid = shm_open(PATH, O_CREAT | O_RDWR, 0666);
    if (shmid == -1)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shmid, sizeof(struct SharedData)) == -1)
    {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    ptr = mmap(0, sizeof(struct SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    data = (struct SharedData *)ptr;

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
            data->count = rand() % (MAX_COUNT - 1) + 2;
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

    exit(EXIT_SUCCESS);
}
