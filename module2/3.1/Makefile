CC = gcc
CFLAGS = -c -Wall
LDFLAGS =
SOURCES = main.c source.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = chmod_emulator.out

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

debug: CFLAGS += -g
debug: clean all

clean:
	rm -f $(OBJECTS) 
