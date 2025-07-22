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

int comparePersons(const Person *a, const Person *b)
{
    int last = strcmp(a->lastName, b->lastName);
    if (last != 0)
        return last;
    return strcmp(a->firstName, b->firstName);
}

Node *insertNode(Node *root, Person *p)
{
    if (!root)
    {
        Node *newNode = malloc(sizeof(Node));
        if (!newNode)
            return NULL;
        newNode->person = *p;
        newNode->left = newNode->right = NULL;
        return newNode;
    }

    if (comparePersons(p, &root->person) < 0)
        root->left = insertNode(root->left, p);
    else
        root->right = insertNode(root->right, p);
    return root;
}

Node *findById(Node *root, int id)
{
    if (!root)
        return NULL;
    if (root->person.id == id)
        return root;
    Node *left = findById(root->left, id);
    return left ? left : findById(root->right, id);
}

Node *findMin(Node *root)
{
    while (root && root->left)
        root = root->left;
    return root;
}

Node *deleteById(Node *root, int id, bool *found)
{
    if (!root)
        return NULL;

    if (id < root->person.id)
        root->left = deleteById(root->left, id, found);
    else if (id > root->person.id)
        root->right = deleteById(root->right, id, found);
    else
    {
        *found = true;
        if (!root->left)
        {
            Node *tmp = root->right;
            free(root);
            return tmp;
        }
        else if (!root->right)
        {
            Node *tmp = root->left;
            free(root);
            return tmp;
        }
        else
        {
            Node *minRight = findMin(root->right);
            root->person = minRight->person;
            root->right = deleteById(root->right, minRight->person.id, found);
        }
    }
    return root;
}

void printPerson(const Node *p)
{
    printf("\nID: %d\n", p->person.id);
    printf("\tФамилия: %s\n", p->person.lastName);
    printf("\tИмя: %s\n", p->person.firstName);
    printf("\tДата рождения: %02d.%02d.%04d\n",
           p->person.DateOfBirth.day, p->person.DateOfBirth.month, p->person.DateOfBirth.year);
    if (strlen(p->person.JobInfo.workplace))
        printf("\tМесто работы: %s\n", p->person.JobInfo.workplace);
    if (strlen(p->person.JobInfo.jobTitle))
        printf("\tДолжность: %s\n", p->person.JobInfo.jobTitle);
    for (int i = 0; i < 3; ++i)
        if (strlen(p->person.phoneNumbers[i]))
            printf("\tТелефон (%d/3): %s\n", i + 1, p->person.phoneNumbers[i]);
}

void printPersonShort(const Node *p)
{
    printf("ID: %d | %s %s\n", p->person.id, p->person.lastName, p->person.firstName);
}

void printInOrder(Node *root, int shortOutput)
{
    if (!root)
        return;
    printInOrder(root->left, shortOutput);
    shortOutput ? printPersonShort(root) : printPerson(root);
    printInOrder(root->right, shortOutput);
}

void storeInOrder(Node *root, Node **arr, int *index)
{
    if (!root)
        return;
    storeInOrder(root->left, arr, index);
    arr[(*index)++] = root;
    storeInOrder(root->right, arr, index);
}

Node *buildBalancedTree(Node **arr, int start, int end)
{
    if (start > end)
        return NULL;
    int mid = (start + end) / 2;
    Node *root = arr[mid];
    root->left = buildBalancedTree(arr, start, mid - 1);
    root->right = buildBalancedTree(arr, mid + 1, end);
    return root;
}

Node *balanceTree(Node *root)
{
    if (!root)
        return NULL;
    Node *nodes[1000];
    int count = 0;
    storeInOrder(root, nodes, &count);
    return buildBalancedTree(nodes, 0, count - 1);
}

void printTree(Node *root, int space)
{
    if (root == NULL)
        return;

    space += 10;

    printTree(root->right, space);

    printf("\n");
    for (int i = 10; i < space; i++)
        printf(" ");
    printf("%s %s (ID: %d)\n", root->person.lastName, root->person.firstName, root->person.id);

    printTree(root->left, space);
}

void menu()
{
    int id_helper = 0;
    Person temp;
    Node *root = NULL;
    char choice;

    while (1)
    {
        printf("\nМЕНЮ\n");
        printf("[1] - Вывести список контактов\n");
        printf("[2] - Добавить контакт\n");
        printf("[3] - Редактировать контакт\n");
        printf("[4] - Удалить контакт\n");
        printf("[5] Выполнить балансировку дерева\n");
        printf("[6] Визуализировать дерево контактов\n");
        printf("[0] Выйти из программы\n");

        scanf(" %c", &choice);
        getchar();

        switch (choice)
        {
        case '1':
        {
            if (!root)
            {
                printf("Список пуст.\n");
                break;
            }
            printf("[1] Короткий вывод\n[2] Полный вывод\n");
            int mode;
            scanf("%d", &mode);
            getchar();
            printInOrder(root, mode == 1);
            break;
        }

        case '2':
        {
            memset(&temp, 0, sizeof(Person));
            temp.id = id_helper++;
            printf("Введите имя: ");
            scanf("%19s", temp.firstName);
            printf("Введите фамилию: ");

            scanf("%19s", temp.lastName);
            bool valid = false;
            do
            {
                printf("Дата рождения (дд.мм.гггг): ");
                int d, m;
                short y;
                if (scanf("%d.%d.%hd", &d, &m, &y) != 3 || !isValidDate(d, m, y))
                {
                    printf("Ошибка. Повторите.\n");
                    while (getchar() != '\n')
                        ;
                }
                else
                {
                    temp.DateOfBirth = (Date){d, m, y};
                    valid = true;
                }
            } while (!valid);
            getchar();

            printf("Введите место работы (опционально): ");
            fgets(temp.JobInfo.workplace, MAX_STRING_LENGTH, stdin);
            temp.JobInfo.workplace[strcspn(temp.JobInfo.workplace, "\n")] = 0;

            if (strlen(temp.JobInfo.workplace))
            {
                printf("Введите должность (опционально): ");
                fgets(temp.JobInfo.jobTitle, MAX_STRING_LENGTH, stdin);
                temp.JobInfo.jobTitle[strcspn(temp.JobInfo.jobTitle, "\n")] = 0;
            }

            printf("Введите до 3-х номеров телефона (Enter — пропустить):\n");
            for (int i = 0; i < 3; ++i)
            {
                printf("Номер телефона (%d/3): ", i + 1);
                fgets(temp.phoneNumbers[i], 13, stdin);
                temp.phoneNumbers[i][strcspn(temp.phoneNumbers[i], "\n")] = 0;
                if (strlen(temp.phoneNumbers[i]) == 0)
                    break;
            }

            root = insertNode(root, &temp);
            break;
        }

        case '3':
        {
            printf("ID для редактирования: ");
            int id;
            scanf("%d", &id);
            getchar();
            Node *target = findById(root, id);
            if (!target)
            {
                printf("Контакт не найден.\n");
                break;
            }

            int ch;
            char buf[128];
            do
            {
                printf("\n[1] Имя (%s)\n[2] Фамилия (%s)\n[3] Телефоны\n[0] Назад\nВыбор: ",
                       target->person.firstName, target->person.lastName);
                scanf("%d", &ch);
                getchar();
                if (ch == 1)
                {
                    printf("Новое имя: ");
                    fgets(buf, 128, stdin);
                    buf[strcspn(buf, "\n")] = 0;
                    strcpy(target->person.firstName, buf);
                }
                else if (ch == 2)
                {
                    printf("Новая фамилия: ");
                    fgets(buf, 128, stdin);
                    buf[strcspn(buf, "\n")] = 0;
                    strcpy(target->person.lastName, buf);
                }
                else if (ch == 3)
                {
                    for (int i = 0; i < 3; ++i)
                    {
                        printf("Телефон %d (был %s): ", i + 1, target->person.phoneNumbers[i]);
                        fgets(buf, 128, stdin);
                        buf[strcspn(buf, "\n")] = 0;
                        if (strlen(buf))
                            strcpy(target->person.phoneNumbers[i], buf);
                    }
                }
            } while (ch != 0);
            break;
        }

        case '4':
        {
            printf("ID для удаления: ");
            int id;
            scanf("%d", &id);
            getchar();
            bool found = false;
            root = deleteById(root, id, &found);
            printf(found ? "Удалено\n" : "Контакт не найден\n");
            break;
        }

        case '5':
            root = balanceTree(root);
            printf("Балансировка завершена.\n");
            break;
        case '6':
            if (!root)
                printf("Дерево пусто.\n");
            else
            {
                printf("\nСТРУКТУРА ДЕРЕВА (корень слева):\n");
                printTree(root, 0);
            }
            break;

        case '0':
        {
            Node *stack[1000];
            int top = -1;
            Node *cur = root;
            while (cur || top >= 0)
            {
                while (cur)
                    stack[++top] = cur, cur = cur->left;
                cur = stack[top--];
                Node *tmp = cur;
                cur = cur->right;
                free(tmp);
            }
            return;
        }

        default:
            printf("Неверный пункт\n");
        }
    }
}
