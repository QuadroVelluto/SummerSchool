obj-m += netlink.o

all: module user

module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

user:
	gcc -Wall -o user.out user.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f user.out