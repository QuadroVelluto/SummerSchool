#include "header.h"

bool is_double(const char *s, double *val)
{
    char *end;
    *val = strtod(s, &end);
    return *end == '\0';
}

bool is_integer(const char *s, int *val)
{
    char *end;
    *val = (int)strtol(s, &end, 10);
    return *end == '\0';
}

void process_arguments(char *argv[], bool parent, int argc)
{
    for (int i = parent ? 0 : 1; i < argc; i += 2)
    {
        int int_val;
        double double_val;
        if (is_integer(argv[i], &int_val))
            printf("%d\n", int_val * 2);
        else if (is_double(argv[i], &double_val))
            printf("%.2f\n", double_val * 2);
        else
            printf("%s\n", argv[i]);
    }
}