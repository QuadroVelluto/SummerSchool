#include "header.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{
    fp = fopen("output.dat", "w");
    if (!fp)
    {
        perror("fopen");
        return 1;
    }

    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);

    int counter = 1;
    while (1)
    {
        fprintf(fp, "%d\n", counter++);
        fflush(fp);
        sleep(1);
    }

    return 0;
}