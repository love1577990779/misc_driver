#include <stdio.h>
#include <linux/input.h>
#include <fcntl.h>

struct input_event ev;

int main(int agrc , char* argv[])
{
	int input_fd = -1;
	input_fd = open("/dev/input/event6" , O_RDWR);
	printf("input_fd = %d\r\n" , input_fd);
	read(input_fd , &ev , sizeof(ev));
	printf("ev.type = %d , ev.code = %d\r\n" , ev.type , ev.code);
}







