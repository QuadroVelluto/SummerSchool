#include "header.h"

int main(int argc, char *argv[])
{
    switch (argc)
    {
    case 1:
        printf("ip_mask [адрес шлюза]/маска [количество сгенерированных адресов]\ns");
        return 0;
    case 3:
        return ip_decider(argv[1], argv[2]);;
    default:
        printf("Неверное количество аргументов\n");
        return 1;
    }
}