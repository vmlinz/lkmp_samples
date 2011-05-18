obj-m += hello.o
obj-m += hello_simple.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
install:

	sudo insmod ./hello.ko
uninstall:
	sudo rmmod hello
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
