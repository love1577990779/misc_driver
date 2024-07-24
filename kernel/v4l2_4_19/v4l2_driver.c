
#include <linux/module.h>
#include <linux/init.h>
#include <linux/videodev2.h>
#include <linux/v4l2-dv-timings.h>
#include <media/videobuf2-vmalloc.h>
#include <media/videobuf2-dma-contig.h>
#include <media/v4l2-dv-timings.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-event.h>
#include <linux/fb.h>
#include <linux/workqueue.h>
#include <media/cec.h>
#include <media/videobuf2-v4l2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-ctrls.h>
#include <media/tpg/v4l2-tpg.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/videodev2.h>
#include <media/v4l2-event.h>
#include <media/v4l2-common.h>



static long my_video_ioctl2(struct file *file,
	       unsigned int cmd, unsigned long arg)
{
	printk(KERN_ALERT "my_video_ioctl2 : enter!\r\n");
	//return video_usercopy(file, cmd, arg, __video_do_ioctl);
	return 0;
}



struct sample_v4l2
{
	struct v4l2_device v4l2_dev;
	struct device dev;
	struct device_driver drv;
	struct v4l2_ctrl_handler sample_handler;
	struct v4l2_ctrl_config sample_ctrl_class;
	struct v4l2_ctrl_ops sample_ctrl_ops;
	struct vb2_queue sample_queue;
	struct vb2_ops sample_vb2_queue_ops;
	struct vb2_mem_ops sample_vb2_queue_mem_ops;
	struct video_device sample_video_dev;
	struct v4l2_file_operations sample_vdev_ops;
};

struct sample_v4l2 sample_v4l2_dev;

static int sample_querycap(struct file *file, void  *priv,
					struct v4l2_capability *cap)
{
	//struct vivid_dev *dev = video_drvdata(file);


	printk(KERN_ALERT "sample_querycap : enter!\r\n");
	cap->capabilities = V4L2_CAP_STREAMING | V4L2_CAP_READWRITE | V4L2_CAP_VIDEO_CAPTURE;
	return 0;
}






static int sample_g_fmt_cap(struct file *file, void *priv,
			   struct v4l2_format *f)
{
	printk(KERN_ALERT "sample_g_fmt_cap : enter!\r\n");
	return 0;
}




static const struct v4l2_ioctl_ops sample_v4l2_ioctl_ops = {
	.vidioc_reqbufs		= vb2_ioctl_reqbufs,
	.vidioc_querycap = sample_querycap ,
	//.vidioc_reqbufs		= clone_vb2_ioctl_reqbufs ,
	.vidioc_g_fmt_vid_cap		= sample_g_fmt_cap,
	.vidioc_querybuf		= vb2_ioctl_querybuf,
	.vidioc_qbuf			= vb2_ioctl_qbuf,
	.vidioc_streamon		= vb2_ioctl_streamon,
	.vidioc_dqbuf			= vb2_ioctl_dqbuf,
};

long my_ioctl2(struct file *file,
	       unsigned int cmd, unsigned long arg)
{
	printk(KERN_ALERT "my_ioctl2 : enter!\r\n");
	return 0;
}

int sample_s_ctrl(struct v4l2_ctrl *ctrl)
{
	printk(KERN_ALERT "sample_s_ctrl : enter!\r\n");
	return 0;
}

void init_ctrl_ops(void)
{
	sample_v4l2_dev.sample_ctrl_ops.s_ctrl = sample_s_ctrl;
}

void init_ctrl_config(void)
{
	sample_v4l2_dev.sample_ctrl_class.ops = &sample_v4l2_dev.sample_ctrl_ops;
	sample_v4l2_dev.sample_ctrl_class.name = "sample_control";
	sample_v4l2_dev.sample_ctrl_class.type = V4L2_CTRL_TYPE_CTRL_CLASS;
	sample_v4l2_dev.sample_ctrl_class.flags = V4L2_CTRL_FLAG_READ_ONLY | V4L2_CTRL_FLAG_WRITE_ONLY;
	sample_v4l2_dev.sample_ctrl_class.id = (0x00f00000 | 1);
}

int sample_queue_setup(struct vb2_queue *vq,
		       unsigned *nbuffers, unsigned *nplanes,
		       unsigned sizes[], struct device *alloc_devs[])
{
	printk(KERN_ALERT "sample_queue_setup : enter!\r\n");
	*nbuffers = 1;
	*nplanes = 1;
	sizes[0] = 1080;
	return 0;
}


void sample_buf_queue(struct vb2_buffer *vb)
{
	printk(KERN_ALERT "vid_cap_buf_queue : enter!\r\n");
}

static int sample_start_streaming(struct vb2_queue *vq, unsigned count)
{

	//vb2_buffer_done(sample_v4l2_dev.sample_queue.bufs[0] , VB2_BUF_STATE_DONE);
	printk(KERN_ALERT "sample_start_streaming : enter!\r\n");
	return 0;
}


void sample_vb2_queue_ops_init(void)
{
	sample_v4l2_dev.sample_vb2_queue_ops.queue_setup = sample_queue_setup;
	sample_v4l2_dev.sample_vb2_queue_ops.buf_queue = sample_buf_queue;
	sample_v4l2_dev.sample_vb2_queue_ops.start_streaming = sample_start_streaming;
}


void sample_vb2_queue_init(void)
{
	sample_v4l2_dev.sample_queue.ops = &sample_v4l2_dev.sample_vb2_queue_ops;
	sample_v4l2_dev.sample_queue.mem_ops = &vb2_vmalloc_memops;
	sample_v4l2_dev.sample_queue.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	sample_v4l2_dev.sample_queue.io_modes = VB2_MMAP;
}


void sample_vdev_release(struct video_device *vdev)
{
	printk(KERN_ALERT "sample_vdev_release : enter!\r\n");
}

int sample_vdev_open(struct file *file)
{
	printk(KERN_ALERT "sample_vdev_open : enter!\r\n");
	return 0;
}

void sample_vdev_ops_init(void)
{
	//sample_v4l2_dev.sample_vdev_ops.open = sample_vdev_open;
	sample_v4l2_dev.sample_vdev_ops.open = v4l2_fh_open;
	sample_v4l2_dev.sample_vdev_ops.unlocked_ioctl = video_ioctl2;
	sample_v4l2_dev.sample_vdev_ops.mmap = vb2_fop_mmap;
	sample_v4l2_dev.sample_vdev_ops.poll = vb2_fop_poll;
	//sample_v4l2_dev.sample_vdev_ops.unlocked_ioctl = my_video_ioctl2;
}


void sample_vdev_init(void)
{
	sample_v4l2_dev.sample_video_dev.fops = &sample_v4l2_dev.sample_vdev_ops;
	sample_v4l2_dev.sample_video_dev.release = sample_vdev_release;
	sample_v4l2_dev.sample_video_dev.v4l2_dev = &sample_v4l2_dev.v4l2_dev;
	sample_v4l2_dev.sample_video_dev.device_caps = V4L2_CAP_STREAMING | V4L2_CAP_READWRITE | V4L2_CAP_VIDEO_CAPTURE;
	sample_v4l2_dev.sample_video_dev.ioctl_ops = &sample_v4l2_ioctl_ops;
	sample_v4l2_dev.sample_video_dev.queue = &sample_v4l2_dev.sample_queue;
}



static int __init hello_init(void)
{
	int ret = 0;
    printk(KERN_ALERT "Hello, World!\n");
	dev_set_name(&sample_v4l2_dev.dev, "sample_dev");
	device_initialize(&sample_v4l2_dev.dev);
	printk(KERN_ALERT "device_initialize : ret = %d!\n" , ret);

	device_register(&sample_v4l2_dev.dev);
	printk(KERN_ALERT "device_register : enter!\r\n");

	sample_v4l2_dev.drv.name = "sample_drv";
	sample_v4l2_dev.dev.driver = &sample_v4l2_dev.drv;

	ret = v4l2_device_register(&sample_v4l2_dev.dev , &sample_v4l2_dev.v4l2_dev);
	printk(KERN_ALERT "v4l2_device_register : ret = %d!\n" , ret);
	
	ret = v4l2_ctrl_handler_init(&sample_v4l2_dev.sample_handler , 10);
	printk(KERN_ALERT "v4l2_ctrl_handler_init : ret = %d!\n" , ret);

	init_ctrl_ops();
	init_ctrl_config();
	ret = v4l2_ctrl_new_custom(&sample_v4l2_dev.sample_handler , &sample_v4l2_dev.sample_ctrl_class , NULL);
	printk(KERN_ALERT "v4l2_ctrl_new_custom : ret = %d!\n" , ret);
    
	sample_v4l2_dev.v4l2_dev.ctrl_handler = &sample_v4l2_dev.sample_handler;

	sample_vb2_queue_ops_init();
	sample_vb2_queue_init();
	ret = vb2_queue_init(&sample_v4l2_dev.sample_queue);
	printk(KERN_ALERT "vb2_queue_init : ret = %d!\n" , ret);
	printk(KERN_ALERT "sample_v4l2_dev.sample_queue.num_buffers = %d!\n" , sample_v4l2_dev.sample_queue.num_buffers);


	sample_vdev_ops_init();
	sample_vdev_init();
	ret = video_register_device(&sample_v4l2_dev.sample_video_dev , VFL_TYPE_GRABBER , 0);
	//printk(KERN_ALERT "sample_v4l2_dev.sample_video_dev.vfl_dir = %d\r\n" , sample_v4l2_dev.sample_video_dev.vfl_dir);
	//printk(KERN_ALERT "sample_v4l2_dev.sample_video_dev.dev.kobj.name = %s\r\n" , sample_v4l2_dev.sample_video_dev.dev.kobj.name);
	printk(KERN_ALERT "video_register_device : ret = %d!\n" , ret);

	




	return 0;

}
 
static void __exit hello_exit(void)
{
	video_unregister_device(&sample_v4l2_dev.sample_video_dev);
	device_unregister(&sample_v4l2_dev.dev);
    printk(KERN_ALERT "Goodbye, World!\n");
}

module_init(hello_init);
module_exit(hello_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Hello World module");