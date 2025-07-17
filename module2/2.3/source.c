#include "header.h"
#include <stdbool.h>

#define NUM_OF_FUNCS 4

double addition(double x, double y)
{
    return x + y;
}

double subtraction(double x, double y)
{
    return x - y;
}

double multiplication(double x, double y)
{
    return x * y;
}

double division(double x, double y)
{
    return x / y;
}

void menu()
{
    Action actions[NUM_OF_FUNCS] =
        {
            {'+', "Сложение", addition},
            {'-', "Вычитание", subtraction},
            {'*', "Умножение", multiplication},
            {'/', "Деление", division},
        };
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
            return;
        if (choice < 1 || choice > NUM_OF_FUNCS)
            {
                printf("Неверный номер пункта\n");
                continue;
            }

        printf("Введите первый аргумент: ");
        scanf("%lf", &x);
        printf("Введите второй аргумент: ");
        scanf("%lf", &y);

        printf("%lf %c %lf = %lf\n", x, actions[choice - 1].sign, y, actions[choice - 1].function_ptr(x, y));
    }
}