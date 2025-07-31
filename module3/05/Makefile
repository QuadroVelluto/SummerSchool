CC = gcc
CFLAGS = -c -Wall
SOURCES = main.c source.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = sig_test.out

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) sig_test.out
