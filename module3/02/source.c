#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "header.h"

#define MAX_ARGS 64

void run_command(char *line)
{
    char *args[MAX_ARGS];
    int i = 0;

    line[strcspn(line, "\n")] = 0;
    if (strcmp(line, "exit") == 0)
        exit(EXIT_SUCCESS);

    char *token = strtok(line, " ");
    while (token != NULL && i < MAX_ARGS - 1)
    {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    if (args[0] == NULL)
        return;

    pid_t pid;

    switch (pid = fork())
    {
    case -1:
        perror("fork");
        exit(EXIT_FAILURE);
    case 0:
        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    default:
        wait(NULL);
        break;
    }
}
