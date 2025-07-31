#include "header.h"
#include <stdlib.h>

FILE *fp = NULL;

void signal_handler(int signo)
{
    static int sigint_count = 0;
    if (signo == SIGINT)
    {
        sigint_count++;
        fprintf(fp, "Received and handled SIGINT (%d)\n", sigint_count);
        if (sigint_count >= 3)
        {
            fclose(fp);
            exit(EXIT_SUCCESS);
        }
    }
    else if (signo == SIGQUIT)
    {
        fprintf(fp, "Received and handled SIGQUIT\n");
    }
}