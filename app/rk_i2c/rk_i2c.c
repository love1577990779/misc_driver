#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#define IMX415_ADDRESS (0x1A)
#define STANDBY_REGISTER_ADDR (0x3000)


static unsigned char i2c_read(int fd, unsigned char reg, unsigned char * val)
{
    int retries;

    for (retries = 5; retries; retries--)
    {
        if (write(fd, &reg, 1) == 1)
        {
            if (read(fd, val, 1) == 1)
            {
                return 0;
            }

        }

    }

    return - 1;
}


int main(void)
{
	int fd = 0;
	int ret = 0;
	unsigned char register_addr[2] = {0};
	unsigned char write_val = 0;
	unsigned char read_val = 0;
	printf("hello world!\r\n");
	fd = open("/dev/i2c-4" , O_RDWR);
	printf("fd = %d\r\n" , fd);
	ret = ioctl(fd , I2C_SLAVE_FORCE , IMX415_ADDRESS);
	printf("ret = %d\r\n" , ret);

	register_addr[0] = 0x3000;
//#if 0
	register_addr[0] = STANDBY_REGISTER_ADDR>>8;
	register_addr[1] = STANDBY_REGISTER_ADDR;
	ret = write(fd , &register_addr , 2);
	printf("ret = %d\r\n" , ret);
	read(fd , &read_val , 1);
	printf("read_val = %d\r\n" , read_val);

	ret = write(fd , &register_addr , 2);
	write(fd , &write_val , 1);
	read(fd , &read_val , 1);
	printf("read_val = %d\r\n" , read_val);
//#endif





#if 0
	i2c_read(fd , STANDBY_REGISTER_ADDR , &read_val);
	printf("read_val = %d\r\n" , read_val);
#endif

	close(fd);
	return 0;
}