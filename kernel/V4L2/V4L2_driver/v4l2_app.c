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

	printf("v4l2_buffer.length = %d , v4l2_buffer.m.offset = 0x%x\r\n" , v4l2_buffer.length , v4l2_buffer.m.offset);

	addr = mmap(NULL , v4l2_buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, v4l2_buffer.m.offset);

	printf("addr = 0x%x\r\n" , addr);

	//ret = ioctl(fd, VIDIOC_QBUF, &v4l2_buffer);

	//ret = ioctl(fd, VIDIOC_STREAMON, &type);

	//ret = ioctl(fd, VIDIOC_DQBUF, &v4l2_buffer);

	
	printf("ret = %d\r\n" , ret);
}
