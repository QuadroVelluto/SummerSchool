#include "header.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("usage: %s [float, int, string] [float, int, string] ...\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    pid_t pid;
    switch (pid = fork())
    {
    case -1:
        perror("fork");
        exit(EXIT_FAILURE);
    case 0:
        process_arguments(argv, false, argc);
        exit(EXIT_SUCCESS);
    default:
        process_arguments(argv, true, argc);
        exit(EXIT_SUCCESS);
    }
}