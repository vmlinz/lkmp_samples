obj-m += hello.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
install:

	sudo insmod ./hello.ko
uninstall:
	sudo rmmod hello
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
