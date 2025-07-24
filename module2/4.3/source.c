#include "header.h"
#include <stdlib.h>
#include <ctype.h>

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

    if (strlen(p->person.JobInfo.workplace))
        printf("\tМесто работы: %s\n", p->person.JobInfo.workplace);

    if (strlen(p->person.JobInfo.jobTitle))
        printf("\tДолжность: %s\n", p->person.JobInfo.jobTitle);

    for (int i = 0; i < 3; ++i)
        if (strlen(p->person.phoneNumbers[i]))
            printf("\tНомер телефона (%d/3): %s\n", i + 1, p->person.phoneNumbers[i]);
}

void printPersonShort(const struct node *const p)
{
    printf("ID: %d | %s %s\n", p->person.id, p->person.lastName, p->person.firstName);
}

void printInOrder(struct node *root, int mode)
{
    if (!root)
        return;
    printInOrder(root->left, mode);
    mode == 2 ? printPerson(root) : printPersonShort(root);
    printInOrder(root->right, mode);
}

struct node *addContact(struct node *root, Person *p)
{
    if (!root)
    {
        struct node *node = malloc(sizeof(struct node));
        if (!node)
        {
            printf("Ошибка malloc\n");
            return NULL;
        }
        node->person = *p;
        node->left = node->right = NULL;
        return node;
    }
    if (strcmp(p->lastName, root->person.lastName) < 0)
        root->left = addContact(root->left, p);
    else
        root->right = addContact(root->right, p);
    return root;
}

struct node *findMin(struct node *root)
{
    while (root->left)
        root = root->left;
    return root;
}

struct node *deleteNode(struct node *root, struct node *target)
{
    if (!root || !target)
        return root;

    if (root == target)
    {
        if (!root->left)
        {
            struct node *temp = root->right;
            free(root);
            return temp;
        }
        else if (!root->right)
        {
            struct node *temp = root->left;
            free(root);
            return temp;
        }
        struct node *minNode = findMin(root->right);
        root->person = minNode->person;
        root->right = deleteNode(root->right, minNode);
        return root;
    }

    root->left = deleteNode(root->left, target);
    root->right = deleteNode(root->right, target);
    return root;
}

struct node *findContactById(struct node *root, int id)
{
    if (!root)
        return NULL;

    struct node *found = findContactById(root->left, id);
    if (found)
        return found;

    if (root->person.id == id)
        return root;

    return findContactById(root->right, id);
}

void freeTree(struct node *root)
{
    if (!root)
        return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

void printTree(struct node *root, int space)
{
    if (!root)
        return;

    const int COUNT = 5;
    space += COUNT;

    printTree(root->right, space);

    printf("\n");
    for (int i = COUNT; i < space; i++)
        printf(" ");
    printf("ID:%d (%s %s)\n", root->person.id, root->person.lastName, root->person.firstName);

    printTree(root->left, space);
}

void storeInOrder(struct node *root, struct node **arr, int *index)
{
    if (!root)
        return;
    storeInOrder(root->left, arr, index);
    arr[(*index)++] = root;
    storeInOrder(root->right, arr, index);
}

struct node *buildBalanced(struct node **arr, int start, int end)
{
    if (start > end)
        return NULL;
    int mid = (start + end) / 2;
    struct node *root = arr[mid];
    root->left = buildBalanced(arr, start, mid - 1);
    root->right = buildBalanced(arr, mid + 1, end);
    return root;
}

int countNodes(struct node *root)
{
    if (!root)
        return 0;
    return 1 + countNodes(root->left) + countNodes(root->right);
}

struct node *balanceTree(struct node *root)
{
    int count = countNodes(root);
    if (count == 0)
        return NULL;

    struct node **arr = malloc(count * sizeof(struct node *));
    int index = 0;
    storeInOrder(root, arr, &index);

    struct node *newRoot = buildBalanced(arr, 0, count - 1);
    free(arr);
    return newRoot;
}

void menu()
{
    int id_helper = 0;
    Person temp;
    struct node *root = NULL;
    char choice = 0;

    while (true)
    {
        printf("\nМЕНЮ\n");
        printf("[1] - Вывести список контактов\n");
        printf("[2] - Добавить контакт\n");
        printf("[3] - Редактировать контакт\n");
        printf("[4] - Удалить контакт\n");
        printf("[5] - Вывести дерево\n");
        printf("[6] - Балансировать дерево\n");
        printf("[0] - Выйти\n");
        printf("Введите номер пункта меню: ");

        scanf(" %hhd", &choice);
        getchar();

        switch (choice)
        {
        case 1:
            if (root == NULL)
                printf("Список контактов пуст\n");
            else
            {
                printf("[1] Короткий вывод\n[2] Полный вывод\n");
                int mode;
                scanf("%d", &mode);
                getchar();
                printInOrder(root, mode);
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
                    while (getchar() != '\n')
                        ;
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
                fgets(temp.phoneNumbers[i], sizeof(temp.phoneNumbers[i]), stdin);
                temp.phoneNumbers[i][strcspn(temp.phoneNumbers[i], "\n")] = '\0';
                if (strlen(temp.phoneNumbers[i]) == 0)
                    break;
            }

            root = addContact(root, &temp);
            break;

        case 3:

            printf("Введите ID контакта для редактирования: ");
            int id;
            scanf("%d", &id);
            getchar();
            struct node *target = findContactById(root, id);
            if (!target)
            {
                printf("Контакт с ID %d не найден.\n", id);
                break;
            }

            Person editedPerson = target->person;
            root = deleteNode(root, target);

            char buffer[128];
            int field_choice;
            bool validDateEdit = false;

            do
            {
                printf("\nРедактирование контакта (ID: %d):\n", editedPerson.id);
                printf("[1] Имя (%s)\n", editedPerson.firstName);
                printf("[2] Фамилия (%s)\n", editedPerson.lastName);
                printf("[3] Дата рождения (%02d.%02d.%04d)\n",
                       editedPerson.DateOfBirth.day,
                       editedPerson.DateOfBirth.month,
                       editedPerson.DateOfBirth.year);
                printf("[4] Место работы (%s)\n", editedPerson.JobInfo.workplace);
                printf("[5] Должность (%s)\n", editedPerson.JobInfo.jobTitle);
                printf("[6] Номера телефонов\n");
                printf("[0] Завершить и сохранить\n");
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
                        strcpy(editedPerson.firstName, buffer);
                    break;
                case 2:
                    printf("Новая фамилия: ");
                    fgets(buffer, sizeof(buffer), stdin);
                    buffer[strcspn(buffer, "\n")] = '\0';
                    if (strlen(buffer))
                        strcpy(editedPerson.lastName, buffer);
                    break;
                case 3:
                    do
                    {
                        printf("Новая дата рождения (дд.мм.гггг): ");
                        int day, month;
                        short year;
                        if (scanf("%d.%d.%hd", &day, &month, &year) != 3)
                        {
                            printf("Неверный формат!\n");
                            while (getchar() != '\n')
                                ;
                            continue;
                        }

                        if (isValidDate(day, month, year))
                        {
                            editedPerson.DateOfBirth.day = day;
                            editedPerson.DateOfBirth.month = month;
                            editedPerson.DateOfBirth.year = year;
                            validDateEdit = true;
                        }
                        else
                        {
                            printf("Некорректная дата.\n");
                        }
                    } while (!validDateEdit);
                    getchar();
                    break;
                case 4:
                    printf("Новое место работы: ");
                    fgets(buffer, sizeof(buffer), stdin);
                    buffer[strcspn(buffer, "\n")] = '\0';
                    strcpy(editedPerson.JobInfo.workplace, buffer);
                    break;
                case 5:
                    printf("Новая должность: ");
                    fgets(buffer, sizeof(buffer), stdin);
                    buffer[strcspn(buffer, "\n")] = '\0';
                    strcpy(editedPerson.JobInfo.jobTitle, buffer);
                    break;
                case 6:
                    for (int i = 0; i < 3; ++i)
                    {
                        printf("Номер телефона (%d/3) (%s): ", i + 1, editedPerson.phoneNumbers[i]);
                        fgets(buffer, sizeof(buffer), stdin);
                        buffer[strcspn(buffer, "\n")] = '\0';
                        if (strlen(buffer))
                            strcpy(editedPerson.phoneNumbers[i], buffer);
                    }
                    break;
                case 0:
                    break;
                default:
                    printf("Неверный выбор.\n");
                }

            } while (field_choice != 0);

            root = addContact(root, &editedPerson);
            printf("Контакт обновлён.\n");
            break;

        case 4:
            printf("Введите ID контакта: ");
            int del_id;
            scanf("%d", &del_id);
            getchar();
            struct node *targetDel = findContactById(root, del_id);
            if (!targetDel)
            {
                printf("Контакт с ID %d не найден.\n", del_id);
                break;
            }

            root = deleteNode(root, targetDel);
            break;

        case 5:
            if (!root)
                printf("Дерево пусто.\n");
            else
            {
                printf("\nСтруктура дерева (визуализация):\n");
                printTree(root, 0);
            }
            break;
        case 6:
            root = balanceTree(root);
            printf("Дерево сбалансировано\n");
            break;

        case 0:
            freeTree(root);
            return;

        default:
            printf("Неверный номер пункта\n");
            break;
        }
    }
}