#include "header.h"
#include <time.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 3) // Подсказка для запуска
    {
        printf("Usage: %s [file path] [number of lines to generate]\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    int fd = open(argv[1], O_CREAT | O_RDWR, 0666); // Открытие файла для записи и чтения
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    char sem_name[256] = {};
    snprintf(sem_name, sizeof(sem_name), "/%s", argv[1]);
    sem_t *sem_p = sem_open(sem_name, O_CREAT, 0666, 1);
    if (sem_p == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    
    // Это для того, чтобы удостоверится, что первые 8 байт существуют
    long int last_line = 0;
    if (read(fd, &last_line, sizeof(long int)) != sizeof(long int))
    {
        last_line = sizeof(long int); // Начинаем писать строки после заголовка
        lseek(fd, 0, SEEK_SET);
        write(fd, &last_line, sizeof(long int));
    }

    int num_of_lines = atoi(argv[2]); // Получение количества строк для генерации
    srand(time(NULL));

    for (int i = 0; i < num_of_lines; i++)
    {
        int count = (rand() % (MAX_COUNT - 1)) + 2; // Количество чисел в строке
        char line[MAX_LINE] = {};
        
        // Сборка строки из случайных чисел
        for (int j = 0; j < count; j++)
        {
            int rand_val = (rand() % (MAX_VAL - MIN_VAL + 1)) + MIN_VAL;
            char num[16];
            sprintf(num, "%d ", rand_val);
            strcat(line, num);
        }
        strcat(line, "\n");
        
        
        printf("%s", line);            // Вывод строки в консоль для удобства сравнения
        sem_wait(sem_p);
        lseek(fd, 0, SEEK_END);        // Переход в конец файла
        write(fd, line, strlen(line)); // Вывод в файл сгенерированной строки
        
        sem_post(sem_p);

        sleep(1); // Поспи немножко
    }

    close(fd);
    sem_close(sem_p);
    exit(EXIT_SUCCESS);
}