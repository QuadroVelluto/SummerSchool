#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS 64
#define MAX_CMDS 16

void parse_command(char *command, char **args)
{
    int i = 0;
    char *token = strtok(command, " ");
    while (token != NULL && i < MAX_ARGS - 1)
    {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

void run_command(char *line)
{
    char *commands[MAX_CMDS];
    int num_cmds = 0;

    line[strcspn(line, "\n")] = 0;

    if (strcmp(line, "exit") == 0)
        exit(EXIT_SUCCESS);

    char *cmd = strtok(line, "|");
    while (cmd != NULL && num_cmds < MAX_CMDS)
    {
        while (*cmd == ' ')
            cmd++;
        commands[num_cmds++] = cmd;
        cmd = strtok(NULL, "|");
    }

    int pipefds[2 * (num_cmds - 1)];
    for (int i = 0; i < num_cmds - 1; i++)
    {
        if (pipe(pipefds + i * 2) < 0)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_cmds; i++)
    {
        char *args[MAX_ARGS];
        parse_command(commands[i], args);

        pid_t pid = fork();
        if (pid == 0)
        {
            if (i != 0)
            {
                if (dup2(pipefds[(i - 1) * 2], STDIN_FILENO) < 0)
                {
                    perror("dup2 stdin");
                    exit(EXIT_FAILURE);
                }
            }

            if (i != num_cmds - 1)
            {
                if (dup2(pipefds[i * 2 + 1], STDOUT_FILENO) < 0)
                {
                    perror("dup2 stdout");
                    exit(EXIT_FAILURE);
                }
            }

            for (int j = 0; j < 2 * (num_cmds - 1); j++)
            {
                close(pipefds[j]);
            }

            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
        else if (pid < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < 2 * (num_cmds - 1); i++)
    {
        close(pipefds[i]);
    }

    for (int i = 0; i < num_cmds; i++)
    {
        wait(NULL);
    }
}