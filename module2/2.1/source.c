#include "header.h"
#include <stdlib.h>
#include <string.h>

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

struct node *addContact(struct node *current, Person *temp)
{
    current->next = malloc(sizeof(struct node));
    current = current->next;
    *current = (struct node){0};

    strcpy(current->person.firstName, temp->firstName);
    strcpy(current->person.lastName, temp->lastName);
    current->person.DateOfBirth = temp->DateOfBirth;
    current->person.id = temp->id;

    strcpy(current->person.JobInfo.workplace, temp->JobInfo.workplace);
    strcpy(current->person.JobInfo.jobTitle, temp->JobInfo.jobTitle);
    for(int i = 0; i < 3; i++)
        strcpy(current->person.phoneNumbers[i], temp->phoneNumbers[i]);

    current->next = NULL;
    return current;
}

struct node *findContactById(struct node *head, int id)
{
	struct node *iter = head->next;
	while (iter != NULL)
	{
		if (iter->person.id == id)
			return iter;
		iter = iter->next;
	}
	return NULL;
}

bool deleteContact(struct node *head, int id)
{
	struct node *contact = findContactById(head, id);
	if (contact != NULL)
		{
			struct node *temp = contact->next;
			contact->next = temp->next;
			free(temp);
			return true;
		}

    return false;
}

void menu()
{
    int id_helper = 0;
    Person temp;
    struct node *head = malloc(sizeof(struct node));
    struct node *current = head;
    head->next = NULL;

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

            printf("Введите дату рождения (дд.мм.гггг): ");
            scanf("%hhd.%hhd.%hd",
                  &temp.DateOfBirth.day,
                  &temp.DateOfBirth.month,
                  &temp.DateOfBirth.year);
            getchar();

            printf("Введите место работы (опционально): ");
            fgets(temp.JobInfo.workplace, sizeof(temp.JobInfo.workplace), stdin);
            temp.JobInfo.workplace[strcspn(temp.JobInfo.workplace, "\n")] = '\0';

            if(strlen(temp.JobInfo.workplace) > 0)
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
                if (strlen(temp.phoneNumbers[i]) == 0) break;
            }

            current = addContact(current, &temp);
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
            do {
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
                    if (strlen(buffer)) strcpy(target->person.firstName, buffer);
                    break;
                case 2:
                    printf("Новая фамилия: ");
                    fgets(buffer, sizeof(buffer), stdin);
                    buffer[strcspn(buffer, "\n")] = '\0';
                    if (strlen(buffer)) strcpy(target->person.lastName, buffer);
                    break;
                case 3:
                    printf("Новая дата рождения (дд.мм.гггг): ");
                    scanf("%hhd.%hhd.%hd",
                          &target->person.DateOfBirth.day,
                          &target->person.DateOfBirth.month,
                          &target->person.DateOfBirth.year);
                    getchar();
                    break;
                case 4:
                    printf("Новое место работы: ");
                    fgets(buffer, sizeof(buffer), stdin);
                    buffer[strcspn(buffer, "\n")] = '\0';
                    if (strlen(buffer)) strcpy(target->person.JobInfo.workplace, buffer);
                    break;
                case 5:
                    printf("Новая должность: ");
                    fgets(buffer, sizeof(buffer), stdin);
                    buffer[strcspn(buffer, "\n")] = '\0';
                    if (strlen(buffer)) strcpy(target->person.JobInfo.jobTitle, buffer);
                    break;
                case 6:
                    for (int i = 0; i < 3; i++) {
                        printf("Номер телефона (%d/3) (текущий: %s): ", i + 1, target->person.phoneNumbers[i]);
                        fgets(buffer, sizeof(buffer), stdin);
                        buffer[strcspn(buffer, "\n")] = '\0';
                        if (strlen(buffer)) strcpy(target->person.phoneNumbers[i], buffer);
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
            if(deleteContact(head, del_id))
                printf("Контакт %d удален\n", del_id);
            else
                printf("Не удалось найти контакт с ID: %d\n", del_id);
            break;

        case 0:
            current = head;
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