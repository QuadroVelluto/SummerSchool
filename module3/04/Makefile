CC = gcc
CFLAGS = -Wall -g
OBJS = main.o source.o
UTILS = sum.out concat.out maxlen.out

all: anton_shell.out $(UTILS)

anton_shell.out: $(OBJS)
	$(CC) $(CFLAGS) -o anton_shell.out $(OBJS)

main.o: main.c header.h
	$(CC) $(CFLAGS) -c main.c

source.o: source.c header.h
	$(CC) $(CFLAGS) -c source.c

sum.out: sum.c
	$(CC) $(CFLAGS) -o sum.out sum.c

concat.out: concat.c
	$(CC) $(CFLAGS) -o concat.out concat.c

maxlen.out: maxlen.c
	$(CC) $(CFLAGS) -o maxlen.out maxlen.c

clean:
	rm -f *.o anton_shell.out $(UTILS)
