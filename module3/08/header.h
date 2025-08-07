#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define PROJ_ID 'A'
#define MAX_COUNT 5
#define MIN_VAL 0
#define MAX_VAL 10
#define MAX_LINE 512

struct sembuf lock[2] = {{0, 0, 0}, {0, 1, 0}};
struct sembuf unlock = {0, -1, 0};
