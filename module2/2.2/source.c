#include "header.h"
#include <stdbool.h>

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
    unsigned char choice = 0;
    double x = 0, y = 0;
    while (true)
    {
        printf("\nМЕНЮ\n");
        printf("[1] - Cложение\n");
        printf("[2] - Вычитание\n");
        printf("[3] - Умножение\n");
        printf("[4] - Деление\n");
        printf("[0] - Выход\n");
        printf("Введите номер пункта меню: ");

        scanf("%hhd", &choice);
        if(choice == 0) return;

        printf("Введите первый аргумент: ");
        scanf("%lf", &x);
        printf("Введите второй аргумент: ");
        scanf("%lf", &y);
        switch (choice)
        {
        case 1:
            printf("%lf + %lf = %lf\n", x, y, addition(x, y));
            break;
        case 2:
            printf("%lf - %lf = %lf\n", x, y, subtraction(x, y));
            break;
        case 3:
            printf("%lf * %lf = %lf\n", x, y, multiplication(x, y));
            break;
        case 4:
            printf("%lf / %lf = %lf\n", x, y, division(x, y));
            break;
        default:
            printf("Неверный номер пункта\n");
            break;
        }
    }
}