# export arm toochain to path first
MAKE:=make

ifeq ($(ARCH),arm)

MY_KDIR:=/home/vmlinz/Projects/android/aosp/kernel/common
MY_CROSS:=arm-eabi-

else

MY_KDIR:=/lib/modules/$(shell uname -r)/build
MY_CROSS:=

endif

obj-m   := my_hello.o
all:
	CROSS_COMPILE=$(MY_CROSS) $(MAKE) -C $(MY_KDIR) M=$(PWD) modules
clean:
	CROSS_COMPILE=$(MY_CROSS) $(MAKE) -C $(MY_KDIR) M=$(PWD) clean
