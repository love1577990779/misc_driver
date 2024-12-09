
MY_OBJ_TARGET := drm_dummy
obj-m := $(MY_OBJ_TARGET).o

TEST_SB := $(wildcard *.c)

# $(MY_OBJ_TARGET)-objs := drm_dummy.o



all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) 
	gcc -o test_dummy_drm.elf dummy_drm_test.c -I"/usr/include/drm" -O0 -g3 -ldrm

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf ./*.o ./*.elf

test:
	@echo $(obj-m)

	@echo $(my_v4l2_driver-objs)