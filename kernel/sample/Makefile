
MY_OBJ_TARGET := my_v4l2_driver
obj-m := $(MY_OBJ_TARGET).o

TEST_SB := $(wildcard *.c)

SB := sample_v4l2_driver

$(MY_OBJ_TARGET)-objs := $(SB).o



all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) 
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test:
	@echo $(obj-m)

	@echo $(my_v4l2_driver-objs)