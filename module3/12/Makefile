CC = gcc
CFLAGS = -Wall -o

server = server.out
client = client.out

all: $(server) $(client)

$(server): server.c
	$(CC) $(CFLAGS) $(server) server.c

$(client): client.c
	$(CC) $(CFLAGS) $(client) client.c -lreadline

clean:
	rm -f $(server) $(client)
