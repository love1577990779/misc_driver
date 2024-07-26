#include <stdio.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>

int main(void)
{
	int fd = 0;
	int ret = 0;
	struct v4l2_requestbuffers req;
	void* addr;
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fd = open("/dev/video0" , O_RDWR);
	printf("fd = %d\r\n" , fd);

	
	req.count = 1; 						//缓存数量
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(fd , VIDIOC_REQBUFS , &req);

	struct v4l2_buffer v4l2_buffer;
	v4l2_buffer.index = 0; //想要查询的缓存
	v4l2_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_buffer.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(fd , VIDIOC_QUERYBUF , &v4l2_buffer);

	ret = ioctl(fd, VIDIOC_QBUF, &v4l2_buffer);

	ret = ioctl(fd, VIDIOC_STREAMON, &type);


	fd_set read_fds;
	struct timeval tv;
	FD_ZERO(&read_fds);
	FD_SET(fd , &read_fds);
	memset(&tv , 0 , sizeof(tv));
	tv.tv_sec = 20;
	tv.tv_usec = 0;
	ret = select(fd + 1 , &read_fds , NULL , NULL , &tv);


	addr = mmap(NULL , v4l2_buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, v4l2_buffer.m.offset);

	printf("addr = 0x%x , addr[0] = %d\r\n" , addr , ((unsigned char*)addr)[0]);

	ret = ioctl(fd, VIDIOC_DQBUF, &v4l2_buffer);

	printf("ret = %d\r\n" , ret);
}
