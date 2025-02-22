
MY_OBJ_TARGET := framebuffer_dummy
KERN_DIR = /home/zero/kernel/linux-6.6.63
obj-m := $(MY_OBJ_TARGET).o

TEST_SB := $(wildcard *.c)

# $(MY_OBJ_TARGET)-objs := drm_dummy.o



all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD)
	gcc -o framebuffer_test.elf framebuffer_test.c -O0 -g3

clean:
	##make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf ./*.o ./*.elf

test:
	@echo $(obj-m)

	@echo $(my_v4l2_driver-objs)
