all: libstatic.a
	gcc main.c -o static.out -L . -lstatic

libstatic.a:
	gcc -c source.c
	ar rc libstatic.a source.o