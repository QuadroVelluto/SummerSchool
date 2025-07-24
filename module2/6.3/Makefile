LIBS_SO = add.so sub.so mult.so div.so

all: all_libs dynamic_calc.out

all_libs: $(LIBS_SO)

add.so:
	gcc -fPIC -c add.c -o add.o
	gcc -shared -o add.so add.o

sub.so:
	gcc -fPIC -c sub.c -o sub.o
	gcc -shared -o sub.so sub.o

mult.so:
	gcc -fPIC -c mult.c -o mult.o
	gcc -shared -o mult.so mult.o

div.so:
	gcc -fPIC -c div.c -o div.o
	gcc -shared -o div.so div.o

dynamic_calc.out:
	gcc -o dynamic_calc.out main.c -ldl

clean:
	rm *.o *.so *.out