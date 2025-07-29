#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "header.h"

#define MAX_LINE 1024

int main()
{
    char line[MAX_LINE];

    while (true)
    {
        printf("anton_shell > ");
        if (!fgets(line, MAX_LINE, stdin))
            break;
        run_command(line);
    }

    exit(EXIT_SUCCESS);
}
