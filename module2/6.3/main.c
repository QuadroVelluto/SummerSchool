#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_OF_FUNCS 4

typedef double (*function_ptr)(double, double);

typedef struct
{
    char sign;
    char name[20];
    function_ptr ptr;
} Action;

int main()
{
    void *handle[NUM_OF_FUNCS];

    handle[0] = dlopen("./add.so", RTLD_LAZY);
    handle[1] = dlopen("./sub.so", RTLD_LAZY);
    handle[2] = dlopen("./mult.so", RTLD_LAZY);
    handle[3] = dlopen("./div.so", RTLD_LAZY);

    for (int i = 0; i < NUM_OF_FUNCS; i++)
    {
        if (!handle[i])
        {
            fprintf(stderr, "Ошибка загрузки библиотеки: %s\n", dlerror());
            exit(1);
        }
    }

    Action actions[NUM_OF_FUNCS] =
        {
            {'+', "Сложение", (function_ptr)dlsym(handle[0], "addition")},
            {'-', "Вычитание", (function_ptr)dlsym(handle[1], "subtraction")},
            {'*', "Умножение", (function_ptr)dlsym(handle[2], "multiplication")},
            {'/', "Деление", (function_ptr)dlsym(handle[3], "division")},
        };

    for (int i = 0; i < NUM_OF_FUNCS; i++)
        if (!actions[i].ptr)
            fprintf(stderr, "Ошибка получения функции: %s\n", dlerror());

    unsigned char choice = 0;
    double x = 0, y = 0;
    while (true)
    {
        for (int i = 0; i < NUM_OF_FUNCS; i++)
            printf("[%d] - %s\n", i + 1, actions[i].name);
        printf("[0] - Выход\n");

        printf("Введите номер пункта меню: ");
        scanf("%hhd", &choice);

        if (choice == 0)
            return 0;
        if (choice < 1 || choice > NUM_OF_FUNCS)
        {
            printf("Неверный номер пункта\n");
            continue;
        }

        printf("Введите первый аргумент: ");
        scanf("%lf", &x);
        printf("Введите второй аргумент: ");
        scanf("%lf", &y);

        printf("%lf %c %lf = %lf\n", x, actions[choice - 1].sign, y, actions[choice - 1].ptr(x, y));
    }

    for (int i = 0; i < NUM_OF_FUNCS; i++)
        dlclose(handle[i]);

    return 0;
}