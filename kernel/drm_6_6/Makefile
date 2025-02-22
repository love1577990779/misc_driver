
MY_OBJ_TARGET := drm_dummy
KERN_DIR = /home/zero/kernel/linux-6.6.63
obj-m := $(MY_OBJ_TARGET).o

TEST_SB := $(wildcard *.c)

# $(MY_OBJ_TARGET)-objs := drm_dummy.o


##-L/home/crosslib/libdrm/lib
all:
	make -C $(KERN_DIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu-
	aarch64-none-linux-gnu-gcc -o test_dummy_drm.elf dummy_drm_test.c -I"/home/crosslib/libdrm/include" -I"/home/crosslib/libdrm/include/libdrm" -O0 -g3 -L/home/crosslib/libdrm/lib -ldrm -Wl,-rpath,./

clean:
	make -C $(KERN_DIR) M=$(PWD) clean ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu-
	rm -rf ./*.o ./*.elf

test:
	@echo $(obj-m)

	@echo $(my_v4l2_driver-objs)
