CC = gcc
CFLAGS = -Wall -o

PRODUCER = producer.out
CONSUMER = consumer.out

all: $(PRODUCER) $(CONSUMER)

$(PRODUCER): producer.c
	$(CC) $(CFLAGS) $(PRODUCER) producer.c -lrt

$(CONSUMER): consumer.c
	$(CC) $(CFLAGS) $(CONSUMER) consumer.c -lrt

clean:
	rm -f $(PRODUCER) $(CONSUMER)
