#include "header.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

__uint32_t gen_ip()
{
    return ((unsigned int)rand() << 16) | (unsigned int)rand();
}

int ip_decider(char gateway[], char N[])
{
    int hits = 0;
    int n = atoi(N);
    char ip_copy[32];
    strncpy(ip_copy, gateway, sizeof(ip_copy));
    ip_copy[sizeof(ip_copy) - 1] = '\0';
    char *ip_str = strtok(ip_copy, "/");
    char *mask_str = strtok(NULL, "/");

    if (!ip_str || !mask_str)
    {
        printf("Неверный формат gateway (ожидается IP/MASK)\n");
        return -1;
    }

    int mask = atoi(mask_str);
    if (mask < 0 || mask > 32)
    {
        printf("Неверная маска %d\n", mask);
        return -2;
    }

    unsigned int ip = 0;
    char *octet_str = strtok(ip_str, ".");
    for (int i = 0; i < 4; i++)
    {
        if (!octet_str)
        {
            printf("Неверный IP адрес\n");
            return -3;
        }
        int octet = atoi(octet_str);
        if (octet < 0 || octet > 255)
        {
            printf("Неверный IP адрес\n");
            return -4;
        }
        ip = (ip << 8) | octet;
        octet_str = strtok(NULL, ".");
    }

    srand((unsigned int)time(NULL));

    unsigned int bitmask = mask ? ~((1 << (32 - mask)) - 1) : 0;

    for (int i = 0; i < n; i++)
    {
        __uint32_t generated_ip = gen_ip();
        if ((ip & bitmask) == (generated_ip & bitmask))
        {
            hits++;
        }
    }

    printf("hits %d percent %.2f%%\n", hits, 100.0f * hits / n);

    return 0;
}