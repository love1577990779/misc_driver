
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
#include <linux/timer.h>




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
	
	struct v4l2_file_operations sample_vdev_ops;
};

struct sample_v4l2 sample_v4l2_dev = {0};
struct v4l2_format g_v4l2_pix_format = {0};
struct media_device simple_md = {0};
static struct timer_list timer_streamon = {0};
static int timer_streamon_need = 0;
struct vb2_queue* g_vb2_queue = NULL;
struct media_pad simple_pads[2] = {0};
struct v4l2_subdev sd = {0};
struct media_pad sd_pads[6] = {0};

void sample_vdev_release(struct video_device *vdev)
{
	printk(KERN_ALERT "sample_vdev_release : enter!\r\n");
}




static int sample_g_fmt_cap(struct file *file, void *priv,
			   struct v4l2_format *f)
{
	printk(KERN_ALERT "sample_g_fmt_cap : enter!\r\n");
	return 0;
}

int debug_vb2_fop_mmap(struct file *file, struct vm_area_struct *vma)
{
	int ret = 0;
	printk(KERN_ALERT "debug_vb2_fop_mmap enter!\r\n");
	ret = vb2_fop_mmap(file, vma);
	return ret;
}

int simple_enum_fmt_vid_cap(struct file *file , void *fh , struct v4l2_fmtdesc *f)
{
	if(f->index > 1)
		return -1;
	else
		f->pixelformat = V4L2_PIX_FMT_NV12;

	return 0;
}

int simple_set_fmt_vid_cap(struct file *file, void *fh,struct v4l2_format *f)
{
	memcpy(&g_v4l2_pix_format , f , sizeof(sizeof(g_v4l2_pix_format)));
	printk(KERN_ALERT "g_v4l2_pix_format.fmt.pix.width = %d , g_v4l2_pix_format.fmt.pix.height = %d\r\n" , g_v4l2_pix_format.fmt.pix.width , g_v4l2_pix_format.fmt.pix.height);
	return 0;
}

int debug_vb2_ioctl_reqbufs(struct file *file, void *priv,
			  struct v4l2_requestbuffers *p)
{
	int ret = 0;
	//printk(KERN_ALERT "sample_v4l2_dev.sample_queue.num_buffers = %d\r\n" , sample_v4l2_dev.sample_queue.num_buffers);
	ret = vb2_ioctl_reqbufs(file , priv , p);
	//printk(KERN_ALERT "sample_v4l2_dev.sample_queue.num_buffers = %d\r\n" , sample_v4l2_dev.sample_queue.num_buffers);

	return ret;
}

int debug_vb2_ioctl_create_bufs(struct file *file , void *priv , struct v4l2_create_buffers *p)
{
	int ret = 0;
	//printk(KERN_ALERT "before vb2_ioctl_create_bufs , sample_v4l2_dev.sample_queue.num_buffers = %d\r\n" , sample_v4l2_dev.sample_queue.num_buffers);
	ret = vb2_ioctl_create_bufs(file , priv , p);
	//printk(KERN_ALERT "after vb2_ioctl_create_bufs , sample_v4l2_dev.sample_queue.num_buffers = %d\r\n" , sample_v4l2_dev.sample_queue.num_buffers);
	return ret;
}

static int sample_querycap(struct file *file, void  *priv,
					struct v4l2_capability *cap)
{
	//struct vivid_dev *dev = video_drvdata(file);
	printk(KERN_ALERT "sample_querycap : enter!\r\n");
	cap->capabilities = V4L2_CAP_STREAMING | V4L2_CAP_READWRITE | V4L2_CAP_VIDEO_CAPTURE;
	return 0;
}

static const struct v4l2_file_operations simple_v4l2_fops = {
	.owner						= THIS_MODULE,
	.open							= v4l2_fh_open,
	.release					= v4l2_fh_release,
	.read							= vb2_fop_read,
	.write						= vb2_fop_write,
	.poll							= vb2_fop_poll,
	.unlocked_ioctl					= video_ioctl2,
	.mmap							= debug_vb2_fop_mmap,
};

static const struct v4l2_ioctl_ops sample_v4l2_ioctl_ops = {
	.vidioc_enum_fmt_vid_cap = simple_enum_fmt_vid_cap,
	.vidioc_reqbufs		= debug_vb2_ioctl_reqbufs,
	.vidioc_create_bufs = debug_vb2_ioctl_create_bufs,
	.vidioc_querycap = sample_querycap ,
	.vidioc_s_fmt_vid_cap	=	simple_set_fmt_vid_cap,
	.vidioc_g_fmt_vid_cap		= sample_g_fmt_cap,
	.vidioc_querybuf		= vb2_ioctl_querybuf,
	.vidioc_qbuf			= vb2_ioctl_qbuf,
	.vidioc_streamon		= vb2_ioctl_streamon,
	.vidioc_dqbuf			= vb2_ioctl_dqbuf,
};

struct video_device simple_vdev = 
{
	.fops = &simple_v4l2_fops,
	.release = sample_vdev_release,
	.v4l2_dev = &sample_v4l2_dev.v4l2_dev,
	.device_caps = V4L2_CAP_STREAMING | V4L2_CAP_READWRITE | V4L2_CAP_VIDEO_CAPTURE,
	.ioctl_ops = &sample_v4l2_ioctl_ops,
	.queue = &sample_v4l2_dev.sample_queue,

};


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
	printk(KERN_ALERT "sample_queue_setup : enter! *nbuffers = %d , *nplanes = %d , sizes[0] = %d\r\n" , *nbuffers , *nplanes , sizes[0]);
	//*nbuffers = 1;
	*nplanes = 1;
	sizes[0] = 1080;
	return 0;
}


void sample_buf_queue(struct vb2_buffer *vb)
{
	printk(KERN_ALERT "vid_cap_buf_queue : enter!\r\n");
}

void timer_streamon_callback(struct timer_list* timer_list)
{
	struct vb2_buffer* vb = NULL;
	void* addr = NULL;
	#if 0
	if(!list_empty(&vq->queued_list))
	{
		vb = list_first_entry(&vq->queued_list , struct vb2_buffer , queued_entry);
		addr = vq->mem_ops->vaddr(vb->planes[0].mem_priv);
		*((unsigned char*)addr) = 2;
		vb2_buffer_done(vb , VB2_BUF_STATE_DONE);
	}
	#endif


	list_for_each_entry(vb , &g_vb2_queue->queued_list , queued_entry)
	{
		if(VB2_BUF_STATE_DONE != vb->state)
		{
			addr = g_vb2_queue->mem_ops->vaddr(vb->planes[0].mem_priv);
			*((unsigned char*)addr) = 2;
			vb2_buffer_done(vb , VB2_BUF_STATE_DONE);
		}
	}


	// 重新启动定时器，如果需要的话
	if((1 == timer_streamon_need))
		mod_timer(&timer_streamon, jiffies + msecs_to_jiffies(1000));
	else
		del_timer(&timer_streamon);
}


static int sample_start_streaming(struct vb2_queue *vq, unsigned count)
{
	printk(KERN_ALERT "sample_start_streaming : enter!\r\n");

	#if 0
	struct vb2_buffer* vb = NULL;
	void* addr = NULL;
	if(!list_empty(&vq->queued_list))
	{
		vb = list_first_entry(&vq->queued_list , struct vb2_buffer , queued_entry);
		addr = vq->mem_ops->vaddr(vb->planes[0].mem_priv);
		*((unsigned char*)addr) = 2;
	}
	vb2_buffer_done(vb , VB2_BUF_STATE_DONE);
	#endif
	g_vb2_queue = vq;
	timer_streamon_need = 1;
	timer_setup(&timer_streamon, timer_streamon_callback, 0);
	mod_timer(&timer_streamon, jiffies + msecs_to_jiffies(100));
	
	return 0;
}

void simple_stop_streaming(struct vb2_queue *q)
{
	timer_streamon_need = 0;
}


void sample_vb2_queue_ops_init(void)
{
	sample_v4l2_dev.sample_vb2_queue_ops.queue_setup = sample_queue_setup;
	sample_v4l2_dev.sample_vb2_queue_ops.buf_queue = sample_buf_queue;
	sample_v4l2_dev.sample_vb2_queue_ops.start_streaming = sample_start_streaming;
	sample_v4l2_dev.sample_vb2_queue_ops.stop_streaming = simple_stop_streaming;
}


void sample_vb2_queue_init(void)
{
	sample_v4l2_dev.sample_queue.ops = &sample_v4l2_dev.sample_vb2_queue_ops;
	sample_v4l2_dev.sample_queue.mem_ops = &vb2_vmalloc_memops;
	sample_v4l2_dev.sample_queue.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	sample_v4l2_dev.sample_queue.io_modes = VB2_MMAP;
}


int sample_vdev_open(struct file *file)
{
	printk(KERN_ALERT "sample_vdev_open : enter!\r\n");
	return 0;
}

void sample_vdev_ops_init(void)
{

	sample_v4l2_dev.sample_vdev_ops.open = v4l2_fh_open;
	sample_v4l2_dev.sample_vdev_ops.unlocked_ioctl = video_ioctl2;
	sample_v4l2_dev.sample_vdev_ops.mmap = vb2_fop_mmap;
	sample_v4l2_dev.sample_vdev_ops.poll = vb2_fop_poll;

}


#if 0
void sample_vdev_init(void)
{
	sample_v4l2_dev.sample_video_dev.fops = &simple_v4l2_fops;
	sample_v4l2_dev.sample_video_dev.release = sample_vdev_release;
	sample_v4l2_dev.sample_video_dev.v4l2_dev = &sample_v4l2_dev.v4l2_dev;
	sample_v4l2_dev.sample_video_dev.device_caps = V4L2_CAP_STREAMING | V4L2_CAP_READWRITE | V4L2_CAP_VIDEO_CAPTURE;
	sample_v4l2_dev.sample_video_dev.ioctl_ops = &sample_v4l2_ioctl_ops;
	sample_v4l2_dev.sample_video_dev.queue = &sample_v4l2_dev.sample_queue;
}
#endif


static int __init simple_v4l2_init(void)
{
	int ret = 0;

	dev_set_name(&sample_v4l2_dev.dev, "sample_dev");
	device_initialize(&sample_v4l2_dev.dev);

	device_register(&sample_v4l2_dev.dev);

	sample_v4l2_dev.drv.name = "sample_drv";
	sample_v4l2_dev.dev.driver = &sample_v4l2_dev.drv;

	ret = v4l2_device_register(&sample_v4l2_dev.dev , &sample_v4l2_dev.v4l2_dev);


	ret = v4l2_ctrl_handler_init(&sample_v4l2_dev.sample_handler , 10);

	init_ctrl_ops();
	init_ctrl_config();
	ret = v4l2_ctrl_new_custom(&sample_v4l2_dev.sample_handler , &sample_v4l2_dev.sample_ctrl_class , NULL);

    
	sample_v4l2_dev.v4l2_dev.ctrl_handler = &sample_v4l2_dev.sample_handler;

	sample_vb2_queue_ops_init();
	sample_vb2_queue_init();
	ret = vb2_queue_init(&sample_v4l2_dev.sample_queue);
	printk(KERN_ALERT "vb2_queue_init : ret = %d!\n" , ret);
	printk(KERN_ALERT "sample_v4l2_dev.sample_queue.num_buffers = %d!\n" , sample_v4l2_dev.sample_queue.num_buffers);


	sample_vdev_ops_init();
	//sample_vdev_init();
	
	memset(&simple_md , 0 , sizeof(simple_md));
	media_device_init(&simple_md);
	strscpy(simple_md.driver_name , "simple_md_drv_name" , sizeof(simple_md.driver_name));
	strscpy(simple_md.model , "simple_md_model" , sizeof(simple_md.model));
	strscpy(simple_md.serial , "simple_md_serial" , sizeof(simple_md.serial));
	strscpy(simple_md.bus_info , "simple_md_bus" , sizeof(simple_md.bus_info));

	ret = media_device_register(&simple_md);
	printk(KERN_ALERT "media_device_register : ret = %d!\n" , ret);

	strscpy(simple_vdev.name , "simple_vdev" , sizeof(simple_vdev.name));
	sample_v4l2_dev.v4l2_dev.mdev = &simple_md;
	simple_vdev.v4l2_dev = &sample_v4l2_dev.v4l2_dev;
	ret = video_register_device(&simple_vdev , VFL_TYPE_GRABBER , 0);
	simple_vdev.entity.name = "vdev_entity";
	printk(KERN_ALERT "video_register_device : ret = %d!\n" , ret);
	printk(KERN_ALERT "simple_vdev.media_entity.name = %s\r\n" , simple_vdev.entity.name);

	simple_pads[0].flags = MEDIA_PAD_FL_SINK;
	simple_pads[1].flags = MEDIA_PAD_FL_SOURCE;
	ret = media_entity_pads_init(&simple_vdev.entity, ARRAY_SIZE(simple_pads), simple_pads);
	
	//sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	strscpy(sd.name , "simple_sd" , sizeof(sd.name));
	sd.entity.name = "sd_entity";
	ret = v4l2_device_register_subdev(&sample_v4l2_dev.v4l2_dev, &sd);
	printk(KERN_ALERT "v4l2_device_register_subdev : ret = %d!\n" , ret);

	//sd.entity.function = MEDIA_ENT_F_V4L2_SUBDEV_UNKNOWN;
	sd_pads[0].flags = MEDIA_PAD_FL_SINK | MEDIA_PAD_FL_MUST_CONNECT;
	sd_pads[1].flags = MEDIA_PAD_FL_SOURCE | MEDIA_PAD_FL_MUST_CONNECT;
	sd_pads[2].flags = MEDIA_PAD_FL_SOURCE;
	sd_pads[3].flags = MEDIA_PAD_FL_SOURCE;
	sd_pads[4].flags = MEDIA_PAD_FL_SOURCE;
	sd_pads[5].flags = MEDIA_PAD_FL_SOURCE;
	ret = media_entity_pads_init(&sd.entity , ARRAY_SIZE(sd_pads), sd_pads);
	printk(KERN_ALERT "media_entity_pads_init : ret = %d!\n" , ret);

	ret = media_create_pad_link(&sd.entity , 1 , &simple_vdev.entity , 0, MEDIA_LNK_FL_ENABLED);
	printk(KERN_ALERT "media_create_pad_link : ret = %d!\n" , ret);


	ret = v4l2_device_register_subdev_nodes(&sample_v4l2_dev.v4l2_dev);
	printk(KERN_ALERT "v4l2_device_register_subdev_nodes : ret = %d!\n" , ret);

	return 0;
}
 
static void __exit simple_v4l2_exit(void)
{
	media_device_unregister(&simple_md);
	video_unregister_device(&simple_vdev);
	v4l2_device_unregister(&sample_v4l2_dev.v4l2_dev);
	device_unregister(&sample_v4l2_dev.dev);
	del_timer(&timer_streamon);
  printk(KERN_ALERT "Goodbye, World!\n");
}

module_init(simple_v4l2_init);
module_exit(simple_v4l2_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Hello World module");