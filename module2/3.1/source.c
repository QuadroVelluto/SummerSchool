#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>

void binary_representation(char buffer[])
{
    int n = strlen(buffer);
    if (n == 3)
    {
        for (int i = 0; i < 3; i++)
            if (buffer[i] < '0' || buffer[i] > '7')
            {
                printf("Неверный формат прав доступа\n");
                return;
            }

        printf("Бинарное представление: ");
        for (int i = 0; i < 3; i++)
        {
            int digit = buffer[i] - '0';
            printf("%d", (digit & 4) ? 1 : 0);
            printf("%d", (digit & 2) ? 1 : 0);
            printf("%d", (digit & 1) ? 1 : 0);
        }
        printf("\n");
    }
    else if (n == 9)
    {
        for (int i = 0; i < 9; i++)
        {

            if (i % 3 == 0 && buffer[i] != 'r' && buffer[i] != '-')
            {
                printf("Неверный формат прав доступа\n");
                return;
            }
            else if (i % 3 == 1 && buffer[i] != 'w' && buffer[i] != '-')
            {
                printf("Неверный формат прав доступа\n");
                return;
            }
            else if (i % 3 == 2 && buffer[i] != 'x' && buffer[i] != '-')
            {
                printf("Неверный формат прав доступа\n");
                return;
            }
        }

        printf("Бинарное представление: ");
        for (int i = 0; i < 9; i++)
            printf("%c", (buffer[i] != '-') ? '1' : '0');
        printf("\n");
    }
    else
        printf("Неверный формат прав доступа\n");
}

void file_mode(char path[])
{
    struct stat fs;

    if (stat(path, &fs) == -1)
    {
        printf("Ошибка файла\n");
        return;
    }

    mode_t mode = fs.st_mode & 0777;
    printf("OCT: %o\n", mode);

    printf("BIN: ");
    char str_mode[10];
    str_mode[9] = '\0';

    for (int i = 0; i < 9; i++)
    {
        mode_t mask = 1 << (8 - i);

        if (mode & mask)
        {
            printf("1");
            switch (i % 3)
            {
            case 0:
                str_mode[i] = 'r';
                break;
            case 1:
                str_mode[i] = 'w';
                break;
            case 2:
                str_mode[i] = 'x';
                break;
            }
        }
        else
        {
            printf("0");
            str_mode[i] = '-';
        }
    }
    printf("\nSTR: %s\n", str_mode);
}

void mode_change(char mode[], char path[])
{
    struct stat fs;

    if (stat(path, &fs) == -1)
    {
        printf("Ошибка файла\n");
        return;
    }

    mode_t base_mode = fs.st_mode & 0777;
    mode_t new_mode = 0;
    int len = strlen(mode);

    if (len == 3 && mode[0] >= '0' && mode[0] <= '7')
    {
        for (int i = 0; i < 3; i++)
            if (mode[i] < '0' || mode[i] > '7')
            {
                printf("Неверный формат прав доступа\n");
                return;
            }

        sscanf(mode, "%o", &new_mode);
    }

    else if (len == 9 && strspn(mode, "rwx-") == 9)
    {
        for (int i = 0; i < 9; i++)
        {
            if ((i % 3 == 0 && mode[i] != 'r' && mode[i] != '-') ||
                (i % 3 == 1 && mode[i] != 'w' && mode[i] != '-') ||
                (i % 3 == 2 && mode[i] != 'x' && mode[i] != '-'))
            {
                printf("Неверный формат прав доступа\n");
                return;
            }
        }

        for (int i = 0; i < 9; i++)
        {
            if (mode[i] != '-')
                new_mode |= 1 << (8 - i);
        }
    }

    else if (len == 9 && strspn(mode, "01") == 9)
    {
        for (int i = 0; i < 9; i++)
        {
            if (mode[i] == '1')
                new_mode |= 1 << (8 - i);
        }
    }

    else
    {
        new_mode = base_mode;
        int i = 0;

        int user = 0, group = 0, other = 0;
        while (mode[i] == 'u' || mode[i] == 'g' || mode[i] == 'o' || mode[i] == 'a')
        {
            switch (mode[i])
            {
            case 'u':
                user = 1;
                break;
            case 'g':
                group = 1;
                break;
            case 'o':
                other = 1;
                break;
            case 'a':
                user = group = other = 1;
                break;
            }
            i++;
        }

        char op = mode[i++];
        if (op != '+' && op != '-' && op != '=')
        {
            printf("Неверный формат оператора\n");
            return;
        }

        char *perms = &mode[i];

        if (strspn(perms, "rwx") != strlen(perms))
        {
            printf("Неверный набор прав доступа\n");
            return;
        }

        if (op == '=')
        {
            if (user)
                new_mode &= ~(7 << 6);
            if (group)
                new_mode &= ~(7 << 3);
            if (other)
                new_mode &= ~(7);
        }

        for (int j = 0; perms[j]; j++)
        {
            mode_t bit = 0;
            switch (perms[j])
            {
            case 'r':
                bit = 4;
                break;
            case 'w':
                bit = 2;
                break;
            case 'x':
                bit = 1;
                break;
            }

            if (user)
            {
                if (op == '+')
                    new_mode |= bit << 6;
                else if (op == '-')
                    new_mode &= ~(bit << 6);
                else if (op == '=')
                    new_mode |= bit << 6;
            }

            if (group)
            {
                if (op == '+')
                    new_mode |= bit << 3;
                else if (op == '-')
                    new_mode &= ~(bit << 3);
                else if (op == '=')
                    new_mode |= bit << 3;
            }

            if (other)
            {
                if (op == '+')
                    new_mode |= bit;
                else if (op == '-')
                    new_mode &= ~bit;
                else if (op == '=')
                    new_mode |= bit;
            }
        }
    }

    printf("Рассчитанные права доступа:\n");

    printf("OCT: %o\n", new_mode);

    printf("BIN: ");
    for (int i = 8; i >= 0; i--)
        printf("%d", (new_mode & (1 << i)) ? 1 : 0);
    printf("\n");

    printf("STR: ");
    for (int i = 0; i < 9; i++)
    {
        mode_t mask = 1 << (8 - i);
        if (new_mode & mask)
        {
            switch (i % 3)
            {
            case 0:
                printf("r");
                break;
            case 1:
                printf("w");
                break;
            case 2:
                printf("x");
                break;
            }
        }
        else
            printf("-");
    }
    printf("\n");
}

void menu()
{
    char choice = 0;
    char buffer[64];
    char second_buffer[64];
    while (1)
    {
        printf("\nМЕНЮ\n");
        printf("[1] - Вывести битовое представление\n");
        printf("[2] - Вывести права доступа файла\n");
        printf("[3] - Изменить права доступа файла\n");
        printf("[0] - Выйти\n");
        printf("Введите номер пункта меню: ");

        scanf("%hhd", &choice);
        getchar();

        switch (choice)
        {
        case 0:
            return;
        case 1:
            printf("Введите права доступа: ");
            scanf("%s", buffer);
            binary_representation(buffer);
            break;
        case 2:
            printf("Введите путь к файлу: ");
            scanf("%s", buffer);
            file_mode(buffer);
            break;
        case 3:
            printf("Введите права доступа и путь к файлу: ");
            scanf("%s %s", buffer, second_buffer);
            mode_change(buffer, second_buffer);
            break;
        default:
            printf("Неверный пункт меню\n");
            break;
        }
    }
}