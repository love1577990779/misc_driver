# 指定内核源码路径，需要根据实际情况修改
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
 
# 模块名称
MODULE_NAME = v4l2_drv
 
# 编译模块所需的对象文件
EXTRA_CFLAGS := -Wno-incomplete-pointer-types


obj-m = $(MODULE_NAME).o
#$(MODULE_NAME)-objs += videobuf2-v4l2.o
$(MODULE_NAME)-objs += v4l2_driver.o
#$(MODULE_NAME)-objs += videobuf2-core.o
#$(MODULE_NAME)-objs += v4l2-ioctl.o
#$(MODULE_NAME)-objs += v4l2-compat-ioctl32.o
#$(MODULE_NAME)-objs += v4l2-trace.o
#$(MODULE_NAME)-objs += videobuf2-vmalloc.o

# 编译模块
driver:
	make -C $(KERNELDIR) M=$(shell pwd) modules

app:
	gcc -o v4l2_app.elf v4l2_app.c

# 安装模块
install:
	make -C $(KERNELDIR) M=$(shell pwd) modules_install INSTALL_MOD_PATH=$$PWD

# 清理编译生成的文件
clean:
	make -C $(KERNELDIR) M=$(shell pwd) clean
	rm *.o || true
