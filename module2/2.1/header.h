#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_STRING_LENGTH 20

typedef struct
{
	unsigned char day, month;
	unsigned short year;
} Date;

typedef struct
{
	char workplace[MAX_STRING_LENGTH];
	char jobTitle[MAX_STRING_LENGTH];
} Job;

typedef struct
{
	int id;
	char firstName[MAX_STRING_LENGTH];
	char lastName[MAX_STRING_LENGTH];
	Date DateOfBirth;
	Job JobInfo;
	char phoneNumbers[20][13];
} Person;

struct node
{
	Person person;
	struct node *next;
};

void menu();
