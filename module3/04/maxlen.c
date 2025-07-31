#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int maxlen = 0;
    char *maxstr = NULL;
    for (int i = 1; i < argc; i++)
    {
        int len = strlen(argv[i]);
        if (len > maxlen)
        {
            maxlen = len;
            maxstr = argv[i];
        }
    }
    if (maxstr)
        printf("Longest: %s (%d chars)\n", maxstr, maxlen);
    return 0;
}
