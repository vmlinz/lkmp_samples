obj-m += lkmp_proc.o
obj-m += lkmp_proc_fops.o
obj-m += lkmp_proc_seq.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
