#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>

#define MAX_COUNT 5
#define MIN_VAL 0
#define MAX_VAL 10
#define MAX_LINE 512
