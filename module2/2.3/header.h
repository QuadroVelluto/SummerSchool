#include <stdio.h>

typedef struct 
{
    char sign;
    char name[20];
    double (*function_ptr) (double, double);
} Action;


void menu(); 