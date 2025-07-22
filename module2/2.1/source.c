#include "header.h"
#include <stdlib.h>
#include <string.h>

bool isValidDate(int day, int month, int year)
{
    if (year < 1900 || year > 2025)
        return false;

    if (month < 1 || month > 12)
        return false;

    int max_day = 31;
    if (month == 4 || month == 6 || month == 9 || month == 11)
        max_day = 30;
    else if (month == 2)
        max_day = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28;

    return (day >= 1 && day <= max_day);
}

void printPerson(const struct node *const p)
{
    printf("\nID: %d\n", p->person.id);
    printf("\tФамилия: %s\n", p->person.lastName);
    printf("\tИмя: %s\n", p->person.firstName);
    printf("\tДата рождения: %02d.%02d.%04d\n",
           p->person.DateOfBirth.day,
           p->person.DateOfBirth.month,
           p->person.DateOfBirth.year);

    if (strlen(p->person.JobInfo.workplace) > 0)
        printf("\tМесто работы: %s\n", p->person.JobInfo.workplace);

    if (strlen(p->person.JobInfo.jobTitle) > 0)
        printf("\tДолжность: %s\n", p->person.JobInfo.jobTitle);

    for (int i = 0; i < 3; ++i)
        if (strlen(p->person.phoneNumbers[i]) > 0)
            printf("\tНомер телефона (%d/3): %s\n", i + 1, p->person.phoneNumbers[i]);
}

void printPersonShort(const struct node *const p)
{
    printf("ID: %d | %s %s\n", p->person.id, p->person.lastName, p->person.firstName);
}

void addContact(struct node *head, Person *temp)
{
    struct node *last = head;
    while (last->next != NULL)
        last = last->next;

    last->next = malloc(sizeof(struct node));
    struct node *newNode = last->next;
    memset(newNode, 0, sizeof(struct node));

    strcpy(newNode->person.firstName, temp->firstName);
    strcpy(newNode->person.lastName, temp->lastName);
    newNode->person.DateOfBirth = temp->DateOfBirth;
    newNode->person.id = temp->id;
    strcpy(newNode->person.JobInfo.workplace, temp->JobInfo.workplace);
    strcpy(newNode->person.JobInfo.jobTitle, temp->JobInfo.jobTitle);
    for (int i = 0; i < 3; i++)
        strcpy(newNode->person.phoneNumbers[i], temp->phoneNumbers[i]);

    newNode->next = NULL;
}

struct node *findContactById(struct node *head, int id)
{
    struct node *iter = head->next;
    for (; iter != NULL; iter = iter->next)
        if (iter->person.id == id)
            return iter;
    return NULL;
}

bool deleteContact(struct node *head, int id)
{
    struct node *prev = head;
    struct node *current = head->next;

    while (current != NULL)
    {
        if (current->person.id == id)
        {
            prev->next = current->next;
            free(current);
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

void menu()
{
    int id_helper = 0;
    Person temp;
    struct node *head = malloc(sizeof(struct node));
    head->next = NULL;
    memset(head, 0, sizeof(struct node));

    char choice = 0;

    while (1)
    {
        printf("\nМЕНЮ\n");
        printf("[1] - Вывести список контактов\n");
        printf("[2] - Добавить контакт\n");
        printf("[3] - Редактировать контакт\n");
        printf("[4] - Удалить контакт\n");
        printf("[0] - Выйти\n");
        printf("Введите номер пункта меню: ");

        scanf("%hhd", &choice);
        getchar();

        switch (choice)
        {
        case 1:
            if (head->next == NULL)
                printf("Список контактов пуст\n");
            else
            {
                printf("[1] Короткий вывод\n[2] Полный вывод\n");
                int mode = 0;
                scanf("%d", &mode);
                getchar();
                struct node *tempNode = head->next;
                while (tempNode != NULL)
                {
                    if (mode == 1)
                        printPersonShort(tempNode);
                    else
                        printPerson(tempNode);
                    tempNode = tempNode->next;
                }
            }
            break;

        case 2:
            memset(&temp, 0, sizeof(Person));
            temp.id = id_helper++;

            printf("Введите имя: ");
            scanf("%s", temp.firstName);

            printf("Введите фамилию: ");
            scanf("%s", temp.lastName);

            bool validDate = false;
            do
            {
                printf("Введите дату рождения (дд.мм.гггг): ");
                int day, month;
                short year;
                if (scanf("%d.%d.%hd", &day, &month, &year) != 3)
                {
                    printf("Неверный формат даты!\n");
                    while (getchar() != '\n');
                    continue;
                }

                if (isValidDate(day, month, year))
                {
                    temp.DateOfBirth.day = day;
                    temp.DateOfBirth.month = month;
                    temp.DateOfBirth.year = year;
                    validDate = true;
                }
                else
                {
                    printf("Некорректная дата!\n");
                }
            } while (!validDate);
            getchar();

            printf("Введите место работы (опционально): ");
            fgets(temp.JobInfo.workplace, sizeof(temp.JobInfo.workplace), stdin);
            temp.JobInfo.workplace[strcspn(temp.JobInfo.workplace, "\n")] = '\0';

            if (strlen(temp.JobInfo.workplace) > 0)
            {
                printf("Введите должность (опционально): ");
                fgets(temp.JobInfo.jobTitle, sizeof(temp.JobInfo.jobTitle), stdin);
                temp.JobInfo.jobTitle[strcspn(temp.JobInfo.jobTitle, "\n")] = '\0';
            }

            printf("Введите до 3-х номеров телефона (Enter — пропустить):\n");
            for (int i = 0; i < 3; ++i)
            {
                printf("Номер телефона (%d/3): ", i + 1);
                fgets(temp.phoneNumbers[i], sizeof(temp.phoneNumbers[0]), stdin);
                temp.phoneNumbers[i][strcspn(temp.phoneNumbers[i], "\n")] = '\0';
                if (strlen(temp.phoneNumbers[i]) == 0)
                    break;
            }

            addContact(head, &temp);
            break;

        case 3:
        {
            printf("Введите ID контакта для редактирования: ");
            int edit_id = 0;
            scanf("%d", &edit_id);
            getchar();
            struct node *target = findContactById(head, edit_id);
            if (!target)
            {
                printf("Контакт с ID %d не найден.\n", edit_id);
                break;
            }

            int field_choice;
            char buffer[128];
            do
            {
                printf("\nЧто изменить?\n");
                printf("[1] Имя (текущее: %s)\n", target->person.firstName);
                printf("[2] Фамилия (текущая: %s)\n", target->person.lastName);
                printf("[3] Дата рождения (текущая: %02d.%02d.%04d)\n",
                       target->person.DateOfBirth.day,
                       target->person.DateOfBirth.month,
                       target->person.DateOfBirth.year);
                printf("[4] Место работы (текущее: %s)\n", target->person.JobInfo.workplace);
                printf("[5] Должность (текущая: %s)\n", target->person.JobInfo.jobTitle);
                printf("[6] Номера телефонов\n");
                printf("[0] Завершить\n");
                printf("Ваш выбор: ");
                scanf("%d", &field_choice);
                getchar();

                switch (field_choice)
                {
                case 1:
                    printf("Новое имя: ");
                    fgets(buffer, sizeof(buffer), stdin);
                    buffer[strcspn(buffer, "\n")] = '\0';
                    if (strlen(buffer))
                        strcpy(target->person.firstName, buffer);
                    break;
                case 2:
                    printf("Новая фамилия: ");
                    fgets(buffer, sizeof(buffer), stdin);
                    buffer[strcspn(buffer, "\n")] = '\0';
                    if (strlen(buffer))
                        strcpy(target->person.lastName, buffer);
                    break;
                case 3:
                    bool validDate = false;
                    do
                    {
                        printf("Новая дата рождения (дд.мм.гггг): ");
                        int day, month;
                        short year;
                        if (scanf("%d.%d.%hd", &day, &month, &year) != 3)
                        {
                            printf("Неверный формат даты!\n");
                            while (getchar() != '\n')
                                ;
                            continue;
                        }

                        if (isValidDate(day, month, year))
                        {
                            target->person.DateOfBirth.day = (char)day;
                            target->person.DateOfBirth.month = (char)month;
                            target->person.DateOfBirth.year = year;
                            validDate = true;
                        }
                        else
                        {
                            printf("Некорректная дата!\n");
                        }
                    } while (!validDate);
                    getchar();
                    break;
                case 4:
                    printf("Новое место работы: ");
                    fgets(buffer, sizeof(buffer), stdin);
                    buffer[strcspn(buffer, "\n")] = '\0';
                    if (strlen(buffer))
                        strcpy(target->person.JobInfo.workplace, buffer);
                    break;
                case 5:
                    printf("Новая должность: ");
                    fgets(buffer, sizeof(buffer), stdin);
                    buffer[strcspn(buffer, "\n")] = '\0';
                    if (strlen(buffer))
                        strcpy(target->person.JobInfo.jobTitle, buffer);
                    break;
                case 6:
                    for (int i = 0; i < 3; i++)
                    {
                        printf("Номер телефона (%d/3) (текущий: %s): ", i + 1, target->person.phoneNumbers[i]);
                        fgets(buffer, sizeof(buffer), stdin);
                        buffer[strcspn(buffer, "\n")] = '\0';
                        if (strlen(buffer))
                            strcpy(target->person.phoneNumbers[i], buffer);
                    }
                    break;
                case 0:
                    break;
                default:
                    printf("Неверный выбор.\n");
                }
            } while (field_choice != 0);

            printf("Контакт обновлён.\n");
            break;
        }

        case 4:
            printf("Введите ID контакта: ");
            int del_id = 0;
            scanf("%d", &del_id);
            if (deleteContact(head, del_id))
                printf("Контакт %d удален\n", del_id);
            else
                printf("Не удалось найти контакт с ID: %d\n", del_id);
            break;

        case 0:
            struct node *current = head;
            while (current != NULL)
            {
                struct node *tmp = current;
                current = current->next;
                free(tmp);
            }
            return;

        default:
            printf("Неверный номер пункта\n");
            break;
        }
    }
}