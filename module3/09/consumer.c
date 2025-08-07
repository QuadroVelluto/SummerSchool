#include "header.h"
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>

static int fd;
static sem_t *sem;
char sem_name[256] = {};

void cleanup(int sigid) // Функция обработки сигнала ctrl + c
{
    printf("\rExited correctly\n");
    sem_close(sem);
    sem_unlink(sem_name);
    close(fd);
    exit(EXIT_SUCCESS);
}

void process_line(const char *line) // Фукнция обработки строки (нахождение min/max)
{
    int min = MAX_VAL;
    int max = -1;
    int num;
    char copy[MAX_LINE];

    strncpy(copy, line, MAX_LINE);
    copy[MAX_LINE - 1] = '\0';

    char *token = strtok(copy, " ");
    while (token)
    {
        if (sscanf(token, "%d", &num) == 1)
        {
            if (num < min)
                min = num;
            if (num > max)
                max = num;
        }
        token = strtok(NULL, " ");
    }

    printf("Line: %s\n", line);
    printf("\tMin: %d, Max: %d\n", min, max);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, cleanup);
    if (argc != 2) // Подсказка для запуска
    {
        printf("Usage: %s [file path]ы\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    snprintf(sem_name, sizeof(sem_name), "/%s", argv[1]);
    sem = sem_open(sem_name, 0);
    if (sem == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    fd = open(argv[1], O_RDWR); // Открытие файла для записи и чтения
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    long int last_line = 0;
    char c;
    char line[MAX_LINE] = {};
    int i = 0;
    ssize_t bytes_read;

    while (1)
    {
        sem_wait(sem);
        lseek(fd, 0, SEEK_SET);                  // Переход в начало файла
        read(fd, &last_line, sizeof(last_line)); // Считывание количества обработанных символов
        lseek(fd, last_line, SEEK_SET);          // Переход на это количество символов вперед

        i = 0;
        while (i < MAX_LINE - 1)
        {
            bytes_read = read(fd, &c, sizeof(c));
            if (bytes_read == 0)
            {
                // Достигнут конец файла - можно подождать и попробовать снова
                goto continue_loop; // Согрешил, сознаюсь
            }
            if (bytes_read < 0)
            {
                perror("read");
                exit(EXIT_FAILURE);
            }

            line[i++] = c;
            if (c == '\n')
                break;
        }
        line[i] = '\0';

        process_line(line); // Обработка строки

        last_line += strlen(line); // Увеличение количества обработанных символов на длину обработанной строки
        lseek(fd, 0, SEEK_SET);
        write(fd, &last_line, sizeof(last_line)); // Перезапись этого числа в начале файла
        continue_loop:
        sem_post(sem);  // Разблокировка семафора
    }

    close(fd);
    exit(EXIT_SUCCESS);
}