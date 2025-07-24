all: source.so
	gcc -o dynamic.out main.c -ldl

source.so:
	gcc -fPIC source.c -c
	gcc -shared source.o -o libcontats.so
