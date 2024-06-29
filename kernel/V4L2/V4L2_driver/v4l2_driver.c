
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

#if 0
/////////////////////////////////////////////////////////////////////////

#include <linux/compat.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/version.h>

#include <linux/videodev2.h>

#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-event.h>
#include <media/v4l2-device.h>
#include <media/videobuf2-v4l2.h>
#include <media/v4l2-mc.h>
#include <media/v4l2-mem2mem.h>

#include <trace/events/v4l2.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/freezer.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/slab.h>

static int debug;
module_param(debug, int, 0644);
#define dprintk(q, level, fmt, arg...)					      \
	do {								      \
		if (debug >= level)					      \
			pr_info("vb2-v4l2: [%p] %s: " fmt,		      \
				(q)->name, __func__, ## arg);		      \
	} while (0)

#define call_qop(q, op, args...)					\
	((q)->ops->op ? (q)->ops->op(args) : 0)


#define INFO_FL_PRIO		(1 << 0)
/* This control can be valid if the filehandle passes a control handler. */
#define INFO_FL_CTRL		(1 << 1)
/* Queuing ioctl */
#define INFO_FL_QUEUE		(1 << 2)
/* Always copy back result, even on error */
#define INFO_FL_ALWAYS_COPY	(1 << 3)
/* Zero struct from after the field to the end */
#define INFO_FL_CLEAR(v4l2_struct, field)			\
	((offsetof(struct v4l2_struct, field) +			\
	  sizeof_field(struct v4l2_struct, field)) << 16)
#define INFO_FL_CLEAR_MASK	(_IOC_SIZEMASK << 16)

#define DEFINE_V4L_STUB_FUNC(_vidioc)				\
	static int v4l_stub_ ## _vidioc(			\
			const struct v4l2_ioctl_ops *ops,	\
			struct file *file, void *fh, void *p)	\
	{							\
		return ops->vidioc_ ## _vidioc(file, fh, p);	\
	}

#define IOCTL_INFO(_ioctl, _func, _debug, _flags)		\
	[_IOC_NR(_ioctl)] = {					\
		.ioctl = _ioctl,				\
		.flags = _flags,				\
		.name = #_ioctl,				\
		.func = _func,					\
		.debug = _debug,				\
	}

#define prt_names(a, arr) (((unsigned)(a)) < ARRAY_SIZE(arr) ? arr[a] : "unknown")

#define CLEAR_AFTER_FIELD(p, field) \
	memset((u8 *)(p) + offsetof(typeof(*(p)), field) + sizeof((p)->field), \
	0, sizeof(*(p)) - offsetof(typeof(*(p)), field) - sizeof((p)->field))

#define is_valid_ioctl(vfd, cmd) test_bit(_IOC_NR(cmd), (vfd)->valid_ioctls)


struct v4l2_ioctl_info {
	unsigned int ioctl;
	u32 flags;
	const char * const name;
	int (*func)(const struct v4l2_ioctl_ops *ops, struct file *file,
		    void *fh, void *p);
	void (*debug)(const void *arg, bool write_only);
};

const char *v4l2_type_names[] = {
	[0]				   = "0",
	[V4L2_BUF_TYPE_VIDEO_CAPTURE]      = "vid-cap",
	[V4L2_BUF_TYPE_VIDEO_OVERLAY]      = "vid-overlay",
	[V4L2_BUF_TYPE_VIDEO_OUTPUT]       = "vid-out",
	[V4L2_BUF_TYPE_VBI_CAPTURE]        = "vbi-cap",
	[V4L2_BUF_TYPE_VBI_OUTPUT]         = "vbi-out",
	[V4L2_BUF_TYPE_SLICED_VBI_CAPTURE] = "sliced-vbi-cap",
	[V4L2_BUF_TYPE_SLICED_VBI_OUTPUT]  = "sliced-vbi-out",
	[V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY] = "vid-out-overlay",
	[V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE] = "vid-cap-mplane",
	[V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE] = "vid-out-mplane",
	[V4L2_BUF_TYPE_SDR_CAPTURE]        = "sdr-cap",
	[V4L2_BUF_TYPE_SDR_OUTPUT]         = "sdr-out",
	[V4L2_BUF_TYPE_META_CAPTURE]       = "meta-cap",
	[V4L2_BUF_TYPE_META_OUTPUT]	   = "meta-out",
};

static const char *v4l2_memory_names[] = {
	[V4L2_MEMORY_MMAP]    = "mmap",
	[V4L2_MEMORY_USERPTR] = "userptr",
	[V4L2_MEMORY_OVERLAY] = "overlay",
	[V4L2_MEMORY_DMABUF] = "dmabuf",
};

struct v4l2_window32 {
	struct v4l2_rect        w;
	__u32			field;	/* enum v4l2_field */
	__u32			chromakey;
	compat_caddr_t		clips; /* actually struct v4l2_clip32 * */
	__u32			clipcount;
	compat_caddr_t		bitmap;
	__u8                    global_alpha;
};

static int get_v4l2_window32(struct v4l2_window *p64,
			     struct v4l2_window32 __user *p32)
{
	struct v4l2_window32 w32;

	if (copy_from_user(&w32, p32, sizeof(w32)))
		return -EFAULT;

	*p64 = (struct v4l2_window) {
		.w		= w32.w,
		.field		= w32.field,
		.chromakey	= w32.chromakey,
		.clips		= (void __force *)compat_ptr(w32.clips),
		.clipcount	= w32.clipcount,
		.bitmap		= compat_ptr(w32.bitmap),
		.global_alpha	= w32.global_alpha,
	};

	if (p64->clipcount > 2048)
		return -EINVAL;
	if (!p64->clipcount)
		p64->clips = NULL;

	return 0;
}

static int put_v4l2_window32(struct v4l2_window *p64,
			     struct v4l2_window32 __user *p32)
{
	struct v4l2_window32 w32;

	memset(&w32, 0, sizeof(w32));
	w32 = (struct v4l2_window32) {
		.w		= p64->w,
		.field		= p64->field,
		.chromakey	= p64->chromakey,
		.clips		= (uintptr_t)p64->clips,
		.clipcount	= p64->clipcount,
		.bitmap		= ptr_to_compat(p64->bitmap),
		.global_alpha	= p64->global_alpha,
	};

	/* copy everything except the clips pointer */
	if (copy_to_user(p32, &w32, offsetof(struct v4l2_window32, clips)) ||
	    copy_to_user(&p32->clipcount, &w32.clipcount,
			 sizeof(w32) - offsetof(struct v4l2_window32, clipcount)))
		return -EFAULT;

	return 0;
}



struct v4l2_format32 {
	__u32	type;	/* enum v4l2_buf_type */


	union {
		struct v4l2_pix_format	pix;
		struct v4l2_pix_format_mplane	pix_mp;
		struct v4l2_window32	win;
		struct v4l2_vbi_format	vbi;
		struct v4l2_sliced_vbi_format	sliced;
		struct v4l2_sdr_format	sdr;
		struct v4l2_meta_format	meta;
		__u8	raw_data[200];        /* user-defined */
	} fmt;


};


struct v4l2_create_buffers32 {
	__u32			index;
	__u32			count;
	__u32			memory;	/* enum v4l2_memory */
	struct v4l2_format32	format;
	__u32			capabilities;
	__u32			reserved[7];
};

struct v4l2_framebuffer32 {
	__u32			capability;
	__u32			flags;
	compat_caddr_t		base;
	struct {
		__u32		width;
		__u32		height;
		__u32		pixelformat;
		__u32		field;
		__u32		bytesperline;
		__u32		sizeimage;
		__u32		colorspace;
		__u32		priv;
	} fmt;
};

struct v4l2_clip32 {
	struct v4l2_rect        c;
	compat_caddr_t		next;
};



struct v4l2_edid32 {
	__u32 pad;
	__u32 start_block;
	__u32 blocks;
	__u32 reserved[5];
	compat_caddr_t edid;
};

static int get_v4l2_edid32(struct v4l2_edid *p64,
			   struct v4l2_edid32 __user *p32)
{
	compat_uptr_t edid;

	if (copy_from_user(p64, p32, offsetof(struct v4l2_edid32, edid)) ||
	    get_user(edid, &p32->edid))
		return -EFAULT;

	p64->edid = (void __force *)compat_ptr(edid);
	return 0;
}

static int put_v4l2_edid32(struct v4l2_edid *p64,
			   struct v4l2_edid32 __user *p32)
{
	if (copy_to_user(p32, p64, offsetof(struct v4l2_edid32, edid)))
		return -EFAULT;
	return 0;
}

#ifdef CONFIG_X86_64
/*
 * x86 is the only compat architecture with different struct alignment
 * between 32-bit and 64-bit tasks.
 */
struct v4l2_event32 {
	__u32				type;
	union {
		compat_s64		value64;
		__u8			data[64];
	} u;
	__u32				pending;
	__u32				sequence;
	struct {
		compat_s64		tv_sec;
		compat_s64		tv_nsec;
	} timestamp;
	__u32				id;
	__u32				reserved[8];
};

static int put_v4l2_event32(struct v4l2_event *p64,
			    struct v4l2_event32 __user *p32)
{
	if (put_user(p64->type, &p32->type) ||
	    copy_to_user(&p32->u, &p64->u, sizeof(p64->u)) ||
	    put_user(p64->pending, &p32->pending) ||
	    put_user(p64->sequence, &p32->sequence) ||
	    put_user(p64->timestamp.tv_sec, &p32->timestamp.tv_sec) ||
	    put_user(p64->timestamp.tv_nsec, &p32->timestamp.tv_nsec) ||
	    put_user(p64->id, &p32->id) ||
	    copy_to_user(p32->reserved, p64->reserved, sizeof(p32->reserved)))
		return -EFAULT;
	return 0;
}

#endif



#define VIDIOC_G_FMT32		_IOWR('V',  4, struct v4l2_format32)
#define VIDIOC_S_FMT32		_IOWR('V',  5, struct v4l2_format32)
#define VIDIOC_QUERYBUF32	_IOWR('V',  9, struct v4l2_buffer32)
#define VIDIOC_G_FBUF32		_IOR ('V', 10, struct v4l2_framebuffer32)
#define VIDIOC_S_FBUF32		_IOW ('V', 11, struct v4l2_framebuffer32)
#define VIDIOC_QBUF32		_IOWR('V', 15, struct v4l2_buffer32)
#define VIDIOC_DQBUF32		_IOWR('V', 17, struct v4l2_buffer32)
#define VIDIOC_ENUMSTD32	_IOWR('V', 25, struct v4l2_standard32)
#define VIDIOC_ENUMINPUT32	_IOWR('V', 26, struct v4l2_input32)
#define VIDIOC_G_EDID32		_IOWR('V', 40, struct v4l2_edid32)
#define VIDIOC_S_EDID32		_IOWR('V', 41, struct v4l2_edid32)
#define VIDIOC_TRY_FMT32	_IOWR('V', 64, struct v4l2_format32)
#define VIDIOC_G_EXT_CTRLS32    _IOWR('V', 71, struct v4l2_ext_controls32)
#define VIDIOC_S_EXT_CTRLS32    _IOWR('V', 72, struct v4l2_ext_controls32)
#define VIDIOC_TRY_EXT_CTRLS32  _IOWR('V', 73, struct v4l2_ext_controls32)
#define	VIDIOC_DQEVENT32	_IOR ('V', 89, struct v4l2_event32)
#define VIDIOC_CREATE_BUFS32	_IOWR('V', 92, struct v4l2_create_buffers32)
#define VIDIOC_PREPARE_BUF32	_IOWR('V', 93, struct v4l2_buffer32)

#ifdef CONFIG_COMPAT_32BIT_TIME
#define VIDIOC_QUERYBUF32_TIME32	_IOWR('V',  9, struct v4l2_buffer32_time32)
#define VIDIOC_QBUF32_TIME32		_IOWR('V', 15, struct v4l2_buffer32_time32)
#define VIDIOC_DQBUF32_TIME32		_IOWR('V', 17, struct v4l2_buffer32_time32)
#define	VIDIOC_DQEVENT32_TIME32		_IOR ('V', 89, struct v4l2_event32_time32)
#define VIDIOC_PREPARE_BUF32_TIME32	_IOWR('V', 93, struct v4l2_buffer32_time32)
#endif

#ifdef CONFIG_COMPAT_32BIT_TIME
struct v4l2_buffer32_time32 {
	__u32			index;
	__u32			type;	/* enum v4l2_buf_type */
	__u32			bytesused;
	__u32			flags;
	__u32			field;	/* enum v4l2_field */
	struct old_timeval32	timestamp;
	struct v4l2_timecode	timecode;
	__u32			sequence;

	/* memory location */
	__u32			memory;	/* enum v4l2_memory */
	union {
		__u32           offset;
		compat_long_t   userptr;
		compat_caddr_t  planes;
		__s32		fd;
	} m;
	__u32			length;
	__u32			reserved2;
	__s32			request_fd;
};
#endif

struct v4l2_buffer32 {
	__u32			index;
	__u32			type;	/* enum v4l2_buf_type */
	__u32			bytesused;
	__u32			flags;
	__u32			field;	/* enum v4l2_field */
	struct {
		compat_s64	tv_sec;
		compat_s64	tv_usec;
	}			timestamp;
	struct v4l2_timecode	timecode;
	__u32			sequence;

	/* memory location */
	__u32			memory;	/* enum v4l2_memory */
	union {
		__u32           offset;
		compat_long_t   userptr;
		compat_caddr_t  planes;
		__s32		fd;
	} m;
	__u32			length;
	__u32			reserved2;
	__s32			request_fd;
};


#ifdef CONFIG_COMPAT_32BIT_TIME
struct v4l2_event32_time32 {
	__u32				type;
	union {
		compat_s64		value64;
		__u8			data[64];
	} u;
	__u32				pending;
	__u32				sequence;
	struct old_timespec32		timestamp;
	__u32				id;
	__u32				reserved[8];
};

static int put_v4l2_event32_time32(struct v4l2_event *p64,
				   struct v4l2_event32_time32 __user *p32)
{
	if (put_user(p64->type, &p32->type) ||
	    copy_to_user(&p32->u, &p64->u, sizeof(p64->u)) ||
	    put_user(p64->pending, &p32->pending) ||
	    put_user(p64->sequence, &p32->sequence) ||
	    put_user(p64->timestamp.tv_sec, &p32->timestamp.tv_sec) ||
	    put_user(p64->timestamp.tv_nsec, &p32->timestamp.tv_nsec) ||
	    put_user(p64->id, &p32->id) ||
	    copy_to_user(p32->reserved, p64->reserved, sizeof(p32->reserved)))
		return -EFAULT;
	return 0;
}
#endif

struct v4l2_ext_controls32 {
	__u32 which;
	__u32 count;
	__u32 error_idx;
	__s32 request_fd;
	__u32 reserved[1];
	compat_caddr_t controls; /* actually struct v4l2_ext_control32 * */
};

struct v4l2_ext_control32 {
	__u32 id;
	__u32 size;
	__u32 reserved2[1];
	union {
		__s32 value;
		__s64 value64;
		compat_caddr_t string; /* actually char * */
	};
} __attribute__ ((packed));

struct v4l2_standard32 {
	__u32		     index;
	compat_u64	     id;
	__u8		     name[24];
	struct v4l2_fract    frameperiod; /* Frames, not fields */
	__u32		     framelines;
	__u32		     reserved[4];
};

static int get_v4l2_standard32(struct v4l2_standard *p64,
			       struct v4l2_standard32 __user *p32)
{
	/* other fields are not set by the user, nor used by the driver */
	return get_user(p64->index, &p32->index);
}

static int put_v4l2_standard32(struct v4l2_standard *p64,
			       struct v4l2_standard32 __user *p32)
{
	if (put_user(p64->index, &p32->index) ||
	    put_user(p64->id, &p32->id) ||
	    copy_to_user(p32->name, p64->name, sizeof(p32->name)) ||
	    copy_to_user(&p32->frameperiod, &p64->frameperiod,
			 sizeof(p32->frameperiod)) ||
	    put_user(p64->framelines, &p32->framelines) ||
	    copy_to_user(p32->reserved, p64->reserved, sizeof(p32->reserved)))
		return -EFAULT;
	return 0;
}

struct v4l2_input32 {
	__u32	     index;		/*  Which input */
	__u8	     name[32];		/*  Label */
	__u32	     type;		/*  Type of input */
	__u32	     audioset;		/*  Associated audios (bitfield) */
	__u32        tuner;             /*  Associated tuner */
	compat_u64   std;
	__u32	     status;
	__u32	     capabilities;
	__u32	     reserved[3];
};

/*
 * The 64-bit v4l2_input struct has extra padding at the end of the struct.
 * Otherwise it is identical to the 32-bit version.
 */
static inline int get_v4l2_input32(struct v4l2_input *p64,
				   struct v4l2_input32 __user *p32)
{
	if (copy_from_user(p64, p32, sizeof(*p32)))
		return -EFAULT;
	return 0;
}

static inline int put_v4l2_input32(struct v4l2_input *p64,
				   struct v4l2_input32 __user *p32)
{
	if (copy_to_user(p32, p64, sizeof(*p32)))
		return -EFAULT;
	return 0;
}




static int check_fmt(struct file *file, enum v4l2_buf_type type)
{

	printk(KERN_ALERT "check_fmt : enter!\r\n");
	const u32 vid_caps = V4L2_CAP_VIDEO_CAPTURE |
			     V4L2_CAP_VIDEO_CAPTURE_MPLANE |
			     V4L2_CAP_VIDEO_OUTPUT |
			     V4L2_CAP_VIDEO_OUTPUT_MPLANE |
			     V4L2_CAP_VIDEO_M2M | V4L2_CAP_VIDEO_M2M_MPLANE;
	const u32 meta_caps = V4L2_CAP_META_CAPTURE |
			      V4L2_CAP_META_OUTPUT;
	struct video_device *vfd = video_devdata(file);
	const struct v4l2_ioctl_ops *ops = vfd->ioctl_ops;


	printk(KERN_ALERT "is_vid : %d %d , ops = %d!\r\n" , vfd->vfl_type == VFL_TYPE_VIDEO , (vfd->device_caps & vid_caps) , (unsigned int)ops);
	bool is_vid = vfd->vfl_type == VFL_TYPE_VIDEO &&
		      (vfd->device_caps & vid_caps);
	bool is_vbi = vfd->vfl_type == VFL_TYPE_VBI;
	bool is_sdr = vfd->vfl_type == VFL_TYPE_SDR;
	bool is_tch = vfd->vfl_type == VFL_TYPE_TOUCH;
	bool is_meta = vfd->vfl_type == VFL_TYPE_VIDEO &&
		       (vfd->device_caps & meta_caps);
	bool is_rx = vfd->vfl_dir != VFL_DIR_TX;
	bool is_tx = vfd->vfl_dir != VFL_DIR_RX;

	if (ops == NULL)
		return -EINVAL;

	printk(KERN_ALERT "type = %d!\r\n" , type);
	switch (type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		printk(KERN_ALERT "type = V4L2_BUF_TYPE_VIDEO_CAPTURE\r\n");
		if ((is_vid || is_tch) && is_rx &&
		    (ops->vidioc_g_fmt_vid_cap || ops->vidioc_g_fmt_vid_cap_mplane))
			return 0;
		break;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		if ((is_vid || is_tch) && is_rx && ops->vidioc_g_fmt_vid_cap_mplane)
			return 0;
		break;
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
		if (is_vid && is_rx && ops->vidioc_g_fmt_vid_overlay)
			return 0;
		break;
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		if (is_vid && is_tx &&
		    (ops->vidioc_g_fmt_vid_out || ops->vidioc_g_fmt_vid_out_mplane))
			return 0;
		break;
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		if (is_vid && is_tx && ops->vidioc_g_fmt_vid_out_mplane)
			return 0;
		break;
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
		if (is_vid && is_tx && ops->vidioc_g_fmt_vid_out_overlay)
			return 0;
		break;
	case V4L2_BUF_TYPE_VBI_CAPTURE:
		if (is_vbi && is_rx && ops->vidioc_g_fmt_vbi_cap)
			return 0;
		break;
	case V4L2_BUF_TYPE_VBI_OUTPUT:
		if (is_vbi && is_tx && ops->vidioc_g_fmt_vbi_out)
			return 0;
		break;
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
		if (is_vbi && is_rx && ops->vidioc_g_fmt_sliced_vbi_cap)
			return 0;
		break;
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		if (is_vbi && is_tx && ops->vidioc_g_fmt_sliced_vbi_out)
			return 0;
		break;
	case V4L2_BUF_TYPE_SDR_CAPTURE:
		if (is_sdr && is_rx && ops->vidioc_g_fmt_sdr_cap)
			return 0;
		break;
	case V4L2_BUF_TYPE_SDR_OUTPUT:
		if (is_sdr && is_tx && ops->vidioc_g_fmt_sdr_out)
			return 0;
		break;
	case V4L2_BUF_TYPE_META_CAPTURE:
		if (is_meta && is_rx && ops->vidioc_g_fmt_meta_cap)
			return 0;
		break;
	case V4L2_BUF_TYPE_META_OUTPUT:
		if (is_meta && is_tx && ops->vidioc_g_fmt_meta_out)
			return 0;
		break;
	default:
		break;
	}


	return -EINVAL;
}




static int v4l_querycap(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct v4l2_capability *cap = (struct v4l2_capability *)arg;
	struct video_device *vfd = video_devdata(file);
	int ret;

	cap->version = LINUX_VERSION_CODE;
	cap->device_caps = vfd->device_caps;
	cap->capabilities = vfd->device_caps | V4L2_CAP_DEVICE_CAPS;

	ret = ops->vidioc_querycap(file, fh, cap);

	/*
	 * Drivers must not change device_caps, so check for this and
	 * warn if this happened.
	 */
	WARN_ON(cap->device_caps != vfd->device_caps);
	/*
	 * Check that capabilities is a superset of
	 * vfd->device_caps | V4L2_CAP_DEVICE_CAPS
	 */
	WARN_ON((cap->capabilities &
		 (vfd->device_caps | V4L2_CAP_DEVICE_CAPS)) !=
		(vfd->device_caps | V4L2_CAP_DEVICE_CAPS));
	cap->capabilities |= V4L2_CAP_EXT_PIX_FORMAT;
	cap->device_caps |= V4L2_CAP_EXT_PIX_FORMAT;

	return ret;
}

static void v4l_print_querycap(const void *arg, bool write_only)
{
	const struct v4l2_capability *p = arg;

	pr_cont("driver=%.*s, card=%.*s, bus=%.*s, version=0x%08x, capabilities=0x%08x, device_caps=0x%08x\n",
		(int)sizeof(p->driver), p->driver,
		(int)sizeof(p->card), p->card,
		(int)sizeof(p->bus_info), p->bus_info,
		p->version, p->capabilities, p->device_caps);
}

static void v4l_print_enuminput(const void *arg, bool write_only)
{
	const struct v4l2_input *p = arg;

	pr_cont("index=%u, name=%.*s, type=%u, audioset=0x%x, tuner=%u, std=0x%08Lx, status=0x%x, capabilities=0x%x\n",
		p->index, (int)sizeof(p->name), p->name, p->type, p->audioset,
		p->tuner, (unsigned long long)p->std, p->status,
		p->capabilities);
}

static void v4l_print_enumoutput(const void *arg, bool write_only)
{
	const struct v4l2_output *p = arg;

	pr_cont("index=%u, name=%.*s, type=%u, audioset=0x%x, modulator=%u, std=0x%08Lx, capabilities=0x%x\n",
		p->index, (int)sizeof(p->name), p->name, p->type, p->audioset,
		p->modulator, (unsigned long long)p->std, p->capabilities);
}

static void v4l_print_audio(const void *arg, bool write_only)
{
	const struct v4l2_audio *p = arg;

	if (write_only)
		pr_cont("index=%u, mode=0x%x\n", p->index, p->mode);
	else
		pr_cont("index=%u, name=%.*s, capability=0x%x, mode=0x%x\n",
			p->index, (int)sizeof(p->name), p->name,
			p->capability, p->mode);
}

static void v4l_print_audioout(const void *arg, bool write_only)
{
	const struct v4l2_audioout *p = arg;

	if (write_only)
		pr_cont("index=%u\n", p->index);
	else
		pr_cont("index=%u, name=%.*s, capability=0x%x, mode=0x%x\n",
			p->index, (int)sizeof(p->name), p->name,
			p->capability, p->mode);
}

static void v4l_print_fmtdesc(const void *arg, bool write_only)
{
	const struct v4l2_fmtdesc *p = arg;

	pr_cont("index=%u, type=%s, flags=0x%x, pixelformat=%p4cc, mbus_code=0x%04x, description='%.*s'\n",
		p->index, prt_names(p->type, v4l2_type_names),
		p->flags, &p->pixelformat, p->mbus_code,
		(int)sizeof(p->description), p->description);
}


const char *v4l2_field_names[] = {
	[V4L2_FIELD_ANY]        = "any",
	[V4L2_FIELD_NONE]       = "none",
	[V4L2_FIELD_TOP]        = "top",
	[V4L2_FIELD_BOTTOM]     = "bottom",
	[V4L2_FIELD_INTERLACED] = "interlaced",
	[V4L2_FIELD_SEQ_TB]     = "seq-tb",
	[V4L2_FIELD_SEQ_BT]     = "seq-bt",
	[V4L2_FIELD_ALTERNATE]  = "alternate",
	[V4L2_FIELD_INTERLACED_TB] = "interlaced-tb",
	[V4L2_FIELD_INTERLACED_BT] = "interlaced-bt",
};


static void v4l_print_format(const void *arg, bool write_only)
{
	const struct v4l2_format *p = arg;
	const struct v4l2_pix_format *pix;
	const struct v4l2_pix_format_mplane *mp;
	const struct v4l2_vbi_format *vbi;
	const struct v4l2_sliced_vbi_format *sliced;
	const struct v4l2_window *win;
	const struct v4l2_meta_format *meta;
	u32 pixelformat;
	u32 planes;
	unsigned i;

	pr_cont("type=%s", prt_names(p->type, v4l2_type_names));
	switch (p->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		pix = &p->fmt.pix;
		pr_cont(", width=%u, height=%u, pixelformat=%p4cc, field=%s, bytesperline=%u, sizeimage=%u, colorspace=%d, flags=0x%x, ycbcr_enc=%u, quantization=%u, xfer_func=%u\n",
			pix->width, pix->height, &pix->pixelformat,
			prt_names(pix->field, v4l2_field_names),
			pix->bytesperline, pix->sizeimage,
			pix->colorspace, pix->flags, pix->ycbcr_enc,
			pix->quantization, pix->xfer_func);
		break;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		mp = &p->fmt.pix_mp;
		pixelformat = mp->pixelformat;
		pr_cont(", width=%u, height=%u, format=%p4cc, field=%s, colorspace=%d, num_planes=%u, flags=0x%x, ycbcr_enc=%u, quantization=%u, xfer_func=%u\n",
			mp->width, mp->height, &pixelformat,
			prt_names(mp->field, v4l2_field_names),
			mp->colorspace, mp->num_planes, mp->flags,
			mp->ycbcr_enc, mp->quantization, mp->xfer_func);
		planes = min_t(u32, mp->num_planes, VIDEO_MAX_PLANES);
		for (i = 0; i < planes; i++)
			printk(KERN_DEBUG "plane %u: bytesperline=%u sizeimage=%u\n", i,
					mp->plane_fmt[i].bytesperline,
					mp->plane_fmt[i].sizeimage);
		break;
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
		win = &p->fmt.win;
		/* Note: we can't print the clip list here since the clips
		 * pointer is a userspace pointer, not a kernelspace
		 * pointer. */
		pr_cont(", wxh=%dx%d, x,y=%d,%d, field=%s, chromakey=0x%08x, clipcount=%u, clips=%p, bitmap=%p, global_alpha=0x%02x\n",
			win->w.width, win->w.height, win->w.left, win->w.top,
			prt_names(win->field, v4l2_field_names),
			win->chromakey, win->clipcount, win->clips,
			win->bitmap, win->global_alpha);
		break;
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
		vbi = &p->fmt.vbi;
		pr_cont(", sampling_rate=%u, offset=%u, samples_per_line=%u, sample_format=%p4cc, start=%u,%u, count=%u,%u\n",
			vbi->sampling_rate, vbi->offset,
			vbi->samples_per_line, &vbi->sample_format,
			vbi->start[0], vbi->start[1],
			vbi->count[0], vbi->count[1]);
		break;
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		sliced = &p->fmt.sliced;
		pr_cont(", service_set=0x%08x, io_size=%d\n",
				sliced->service_set, sliced->io_size);
		for (i = 0; i < 24; i++)
			printk(KERN_DEBUG "line[%02u]=0x%04x, 0x%04x\n", i,
				sliced->service_lines[0][i],
				sliced->service_lines[1][i]);
		break;
	case V4L2_BUF_TYPE_SDR_CAPTURE:
	case V4L2_BUF_TYPE_SDR_OUTPUT:
		pixelformat = p->fmt.sdr.pixelformat;
		pr_cont(", pixelformat=%p4cc\n", &pixelformat);
		break;
	case V4L2_BUF_TYPE_META_CAPTURE:
	case V4L2_BUF_TYPE_META_OUTPUT:
		meta = &p->fmt.meta;
		pixelformat = meta->dataformat;
		pr_cont(", dataformat=%p4cc, buffersize=%u\n",
			&pixelformat, meta->buffersize);
		break;
	}
}

static void v4l_print_framebuffer(const void *arg, bool write_only)
{
	const struct v4l2_framebuffer *p = arg;

	pr_cont("capability=0x%x, flags=0x%x, base=0x%p, width=%u, height=%u, pixelformat=%p4cc, bytesperline=%u, sizeimage=%u, colorspace=%d\n",
		p->capability, p->flags, p->base, p->fmt.width, p->fmt.height,
		&p->fmt.pixelformat, p->fmt.bytesperline, p->fmt.sizeimage,
		p->fmt.colorspace);
}

static void v4l_print_buftype(const void *arg, bool write_only)
{
	pr_cont("type=%s\n", prt_names(*(u32 *)arg, v4l2_type_names));
}

static void v4l_print_modulator(const void *arg, bool write_only)
{
	const struct v4l2_modulator *p = arg;

	if (write_only)
		pr_cont("index=%u, txsubchans=0x%x\n", p->index, p->txsubchans);
	else
		pr_cont("index=%u, name=%.*s, capability=0x%x, rangelow=%u, rangehigh=%u, txsubchans=0x%x\n",
			p->index, (int)sizeof(p->name), p->name, p->capability,
			p->rangelow, p->rangehigh, p->txsubchans);
}

static void v4l_print_tuner(const void *arg, bool write_only)
{
	const struct v4l2_tuner *p = arg;

	if (write_only)
		pr_cont("index=%u, audmode=%u\n", p->index, p->audmode);
	else
		pr_cont("index=%u, name=%.*s, type=%u, capability=0x%x, rangelow=%u, rangehigh=%u, signal=%u, afc=%d, rxsubchans=0x%x, audmode=%u\n",
			p->index, (int)sizeof(p->name), p->name, p->type,
			p->capability, p->rangelow,
			p->rangehigh, p->signal, p->afc,
			p->rxsubchans, p->audmode);
}

static void v4l_print_frequency(const void *arg, bool write_only)
{
	const struct v4l2_frequency *p = arg;

	pr_cont("tuner=%u, type=%u, frequency=%u\n",
				p->tuner, p->type, p->frequency);
}

static void v4l_print_standard(const void *arg, bool write_only)
{
	const struct v4l2_standard *p = arg;

	pr_cont("index=%u, id=0x%Lx, name=%.*s, fps=%u/%u, framelines=%u\n",
		p->index,
		(unsigned long long)p->id, (int)sizeof(p->name), p->name,
		p->frameperiod.numerator,
		p->frameperiod.denominator,
		p->framelines);
}

static void v4l_print_std(const void *arg, bool write_only)
{
	pr_cont("std=0x%08Lx\n", *(const long long unsigned *)arg);
}

static void v4l_print_hw_freq_seek(const void *arg, bool write_only)
{
	const struct v4l2_hw_freq_seek *p = arg;

	pr_cont("tuner=%u, type=%u, seek_upward=%u, wrap_around=%u, spacing=%u, rangelow=%u, rangehigh=%u\n",
		p->tuner, p->type, p->seek_upward, p->wrap_around, p->spacing,
		p->rangelow, p->rangehigh);
}

static void v4l_print_requestbuffers(const void *arg, bool write_only)
{
	const struct v4l2_requestbuffers *p = arg;

	pr_cont("count=%d, type=%s, memory=%s\n",
		p->count,
		prt_names(p->type, v4l2_type_names),
		prt_names(p->memory, v4l2_memory_names));
}

static void v4l_print_buffer(const void *arg, bool write_only)
{
	const struct v4l2_buffer *p = arg;
	const struct v4l2_timecode *tc = &p->timecode;
	const struct v4l2_plane *plane;
	int i;

	pr_cont("%02d:%02d:%02d.%06ld index=%d, type=%s, request_fd=%d, flags=0x%08x, field=%s, sequence=%d, memory=%s",
			(int)p->timestamp.tv_sec / 3600,
			((int)p->timestamp.tv_sec / 60) % 60,
			((int)p->timestamp.tv_sec % 60),
			(long)p->timestamp.tv_usec,
			p->index,
			prt_names(p->type, v4l2_type_names), p->request_fd,
			p->flags, prt_names(p->field, v4l2_field_names),
			p->sequence, prt_names(p->memory, v4l2_memory_names));

	if (V4L2_TYPE_IS_MULTIPLANAR(p->type) && p->m.planes) {
		pr_cont("\n");
		for (i = 0; i < p->length; ++i) {
			plane = &p->m.planes[i];
			printk(KERN_DEBUG
				"plane %d: bytesused=%d, data_offset=0x%08x, offset/userptr=0x%lx, length=%d\n",
				i, plane->bytesused, plane->data_offset,
				plane->m.userptr, plane->length);
		}
	} else {
		pr_cont(", bytesused=%d, offset/userptr=0x%lx, length=%d\n",
			p->bytesused, p->m.userptr, p->length);
	}

	printk(KERN_DEBUG "timecode=%02d:%02d:%02d type=%d, flags=0x%08x, frames=%d, userbits=0x%08x\n",
			tc->hours, tc->minutes, tc->seconds,
			tc->type, tc->flags, tc->frames, *(__u32 *)tc->userbits);
}

static void v4l_print_exportbuffer(const void *arg, bool write_only)
{
	const struct v4l2_exportbuffer *p = arg;

	pr_cont("fd=%d, type=%s, index=%u, plane=%u, flags=0x%08x\n",
		p->fd, prt_names(p->type, v4l2_type_names),
		p->index, p->plane, p->flags);
}

static void v4l_print_create_buffers(const void *arg, bool write_only)
{
	const struct v4l2_create_buffers *p = arg;

	pr_cont("index=%d, count=%d, memory=%s, capabilities=0x%08x, ",
		p->index, p->count, prt_names(p->memory, v4l2_memory_names),
		p->capabilities);
	v4l_print_format(&p->format, write_only);
}

static void v4l_print_streamparm(const void *arg, bool write_only)
{
	const struct v4l2_streamparm *p = arg;

	pr_cont("type=%s", prt_names(p->type, v4l2_type_names));

	if (p->type == V4L2_BUF_TYPE_VIDEO_CAPTURE ||
	    p->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		const struct v4l2_captureparm *c = &p->parm.capture;

		pr_cont(", capability=0x%x, capturemode=0x%x, timeperframe=%d/%d, extendedmode=%d, readbuffers=%d\n",
			c->capability, c->capturemode,
			c->timeperframe.numerator, c->timeperframe.denominator,
			c->extendedmode, c->readbuffers);
	} else if (p->type == V4L2_BUF_TYPE_VIDEO_OUTPUT ||
		   p->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		const struct v4l2_outputparm *c = &p->parm.output;

		pr_cont(", capability=0x%x, outputmode=0x%x, timeperframe=%d/%d, extendedmode=%d, writebuffers=%d\n",
			c->capability, c->outputmode,
			c->timeperframe.numerator, c->timeperframe.denominator,
			c->extendedmode, c->writebuffers);
	} else {
		pr_cont("\n");
	}
}

static void v4l_print_queryctrl(const void *arg, bool write_only)
{
	const struct v4l2_queryctrl *p = arg;

	pr_cont("id=0x%x, type=%d, name=%.*s, min/max=%d/%d, step=%d, default=%d, flags=0x%08x\n",
			p->id, p->type, (int)sizeof(p->name), p->name,
			p->minimum, p->maximum,
			p->step, p->default_value, p->flags);
}

static void v4l_print_query_ext_ctrl(const void *arg, bool write_only)
{
	const struct v4l2_query_ext_ctrl *p = arg;

	pr_cont("id=0x%x, type=%d, name=%.*s, min/max=%lld/%lld, step=%lld, default=%lld, flags=0x%08x, elem_size=%u, elems=%u, nr_of_dims=%u, dims=%u,%u,%u,%u\n",
			p->id, p->type, (int)sizeof(p->name), p->name,
			p->minimum, p->maximum,
			p->step, p->default_value, p->flags,
			p->elem_size, p->elems, p->nr_of_dims,
			p->dims[0], p->dims[1], p->dims[2], p->dims[3]);
}

static void v4l_print_querymenu(const void *arg, bool write_only)
{
	const struct v4l2_querymenu *p = arg;

	pr_cont("id=0x%x, index=%d\n", p->id, p->index);
}

static void v4l_print_control(const void *arg, bool write_only)
{
	const struct v4l2_control *p = arg;
	const char *name = v4l2_ctrl_get_name(p->id);

	if (name)
		pr_cont("name=%s, ", name);
	pr_cont("id=0x%x, value=%d\n", p->id, p->value);
}

static void v4l_print_ext_controls(const void *arg, bool write_only)
{
	const struct v4l2_ext_controls *p = arg;
	int i;

	pr_cont("which=0x%x, count=%d, error_idx=%d, request_fd=%d",
			p->which, p->count, p->error_idx, p->request_fd);
	for (i = 0; i < p->count; i++) {
		unsigned int id = p->controls[i].id;
		const char *name = v4l2_ctrl_get_name(id);

		if (name)
			pr_cont(", name=%s", name);
		if (!p->controls[i].size)
			pr_cont(", id/val=0x%x/0x%x", id, p->controls[i].value);
		else
			pr_cont(", id/size=0x%x/%u", id, p->controls[i].size);
	}
	pr_cont("\n");
}

static void v4l_print_cropcap(const void *arg, bool write_only)
{
	const struct v4l2_cropcap *p = arg;

	pr_cont("type=%s, bounds wxh=%dx%d, x,y=%d,%d, defrect wxh=%dx%d, x,y=%d,%d, pixelaspect %d/%d\n",
		prt_names(p->type, v4l2_type_names),
		p->bounds.width, p->bounds.height,
		p->bounds.left, p->bounds.top,
		p->defrect.width, p->defrect.height,
		p->defrect.left, p->defrect.top,
		p->pixelaspect.numerator, p->pixelaspect.denominator);
}

static void v4l_print_crop(const void *arg, bool write_only)
{
	const struct v4l2_crop *p = arg;

	pr_cont("type=%s, wxh=%dx%d, x,y=%d,%d\n",
		prt_names(p->type, v4l2_type_names),
		p->c.width, p->c.height,
		p->c.left, p->c.top);
}

static void v4l_print_selection(const void *arg, bool write_only)
{
	const struct v4l2_selection *p = arg;

	pr_cont("type=%s, target=%d, flags=0x%x, wxh=%dx%d, x,y=%d,%d\n",
		prt_names(p->type, v4l2_type_names),
		p->target, p->flags,
		p->r.width, p->r.height, p->r.left, p->r.top);
}

static void v4l_print_jpegcompression(const void *arg, bool write_only)
{
	const struct v4l2_jpegcompression *p = arg;

	pr_cont("quality=%d, APPn=%d, APP_len=%d, COM_len=%d, jpeg_markers=0x%x\n",
		p->quality, p->APPn, p->APP_len,
		p->COM_len, p->jpeg_markers);
}

static void v4l_print_enc_idx(const void *arg, bool write_only)
{
	const struct v4l2_enc_idx *p = arg;

	pr_cont("entries=%d, entries_cap=%d\n",
			p->entries, p->entries_cap);
}

static void v4l_print_encoder_cmd(const void *arg, bool write_only)
{
	const struct v4l2_encoder_cmd *p = arg;

	pr_cont("cmd=%d, flags=0x%x\n",
			p->cmd, p->flags);
}

static void v4l_print_decoder_cmd(const void *arg, bool write_only)
{
	const struct v4l2_decoder_cmd *p = arg;

	pr_cont("cmd=%d, flags=0x%x\n", p->cmd, p->flags);

	if (p->cmd == V4L2_DEC_CMD_START)
		pr_info("speed=%d, format=%u\n",
				p->start.speed, p->start.format);
	else if (p->cmd == V4L2_DEC_CMD_STOP)
		pr_info("pts=%llu\n", p->stop.pts);
}

static void v4l_print_dbg_chip_info(const void *arg, bool write_only)
{
	const struct v4l2_dbg_chip_info *p = arg;

	pr_cont("type=%u, ", p->match.type);
	if (p->match.type == V4L2_CHIP_MATCH_I2C_DRIVER)
		pr_cont("name=%.*s, ",
				(int)sizeof(p->match.name), p->match.name);
	else
		pr_cont("addr=%u, ", p->match.addr);
	pr_cont("name=%.*s\n", (int)sizeof(p->name), p->name);
}

static void v4l_print_dbg_register(const void *arg, bool write_only)
{
	const struct v4l2_dbg_register *p = arg;

	pr_cont("type=%u, ", p->match.type);
	if (p->match.type == V4L2_CHIP_MATCH_I2C_DRIVER)
		pr_cont("name=%.*s, ",
				(int)sizeof(p->match.name), p->match.name);
	else
		pr_cont("addr=%u, ", p->match.addr);
	pr_cont("reg=0x%llx, val=0x%llx\n",
			p->reg, p->val);
}

static void v4l_print_dv_timings(const void *arg, bool write_only)
{
	const struct v4l2_dv_timings *p = arg;

	switch (p->type) {
	case V4L2_DV_BT_656_1120:
		pr_cont("type=bt-656/1120, interlaced=%u, pixelclock=%llu, width=%u, height=%u, polarities=0x%x, hfrontporch=%u, hsync=%u, hbackporch=%u, vfrontporch=%u, vsync=%u, vbackporch=%u, il_vfrontporch=%u, il_vsync=%u, il_vbackporch=%u, standards=0x%x, flags=0x%x\n",
				p->bt.interlaced, p->bt.pixelclock,
				p->bt.width, p->bt.height,
				p->bt.polarities, p->bt.hfrontporch,
				p->bt.hsync, p->bt.hbackporch,
				p->bt.vfrontporch, p->bt.vsync,
				p->bt.vbackporch, p->bt.il_vfrontporch,
				p->bt.il_vsync, p->bt.il_vbackporch,
				p->bt.standards, p->bt.flags);
		break;
	default:
		pr_cont("type=%d\n", p->type);
		break;
	}
}

static void v4l_print_enum_dv_timings(const void *arg, bool write_only)
{
	const struct v4l2_enum_dv_timings *p = arg;

	pr_cont("index=%u, ", p->index);
	v4l_print_dv_timings(&p->timings, write_only);
}

static void v4l_print_dv_timings_cap(const void *arg, bool write_only)
{
	const struct v4l2_dv_timings_cap *p = arg;

	switch (p->type) {
	case V4L2_DV_BT_656_1120:
		pr_cont("type=bt-656/1120, width=%u-%u, height=%u-%u, pixelclock=%llu-%llu, standards=0x%x, capabilities=0x%x\n",
			p->bt.min_width, p->bt.max_width,
			p->bt.min_height, p->bt.max_height,
			p->bt.min_pixelclock, p->bt.max_pixelclock,
			p->bt.standards, p->bt.capabilities);
		break;
	default:
		pr_cont("type=%u\n", p->type);
		break;
	}
}

static void v4l_print_frmsizeenum(const void *arg, bool write_only)
{
	const struct v4l2_frmsizeenum *p = arg;

	pr_cont("index=%u, pixelformat=%p4cc, type=%u",
		p->index, &p->pixel_format, p->type);
	switch (p->type) {
	case V4L2_FRMSIZE_TYPE_DISCRETE:
		pr_cont(", wxh=%ux%u\n",
			p->discrete.width, p->discrete.height);
		break;
	case V4L2_FRMSIZE_TYPE_STEPWISE:
		pr_cont(", min=%ux%u, max=%ux%u, step=%ux%u\n",
				p->stepwise.min_width,
				p->stepwise.min_height,
				p->stepwise.max_width,
				p->stepwise.max_height,
				p->stepwise.step_width,
				p->stepwise.step_height);
		break;
	case V4L2_FRMSIZE_TYPE_CONTINUOUS:
	default:
		pr_cont("\n");
		break;
	}
}

static void v4l_print_frmivalenum(const void *arg, bool write_only)
{
	const struct v4l2_frmivalenum *p = arg;

	pr_cont("index=%u, pixelformat=%p4cc, wxh=%ux%u, type=%u",
		p->index, &p->pixel_format, p->width, p->height, p->type);
	switch (p->type) {
	case V4L2_FRMIVAL_TYPE_DISCRETE:
		pr_cont(", fps=%d/%d\n",
				p->discrete.numerator,
				p->discrete.denominator);
		break;
	case V4L2_FRMIVAL_TYPE_STEPWISE:
		pr_cont(", min=%d/%d, max=%d/%d, step=%d/%d\n",
				p->stepwise.min.numerator,
				p->stepwise.min.denominator,
				p->stepwise.max.numerator,
				p->stepwise.max.denominator,
				p->stepwise.step.numerator,
				p->stepwise.step.denominator);
		break;
	case V4L2_FRMIVAL_TYPE_CONTINUOUS:
	default:
		pr_cont("\n");
		break;
	}
}

static void v4l_print_event(const void *arg, bool write_only)
{
	const struct v4l2_event *p = arg;
	const struct v4l2_event_ctrl *c;

	pr_cont("type=0x%x, pending=%u, sequence=%u, id=%u, timestamp=%llu.%9.9llu\n",
			p->type, p->pending, p->sequence, p->id,
			p->timestamp.tv_sec, p->timestamp.tv_nsec);
	switch (p->type) {
	case V4L2_EVENT_VSYNC:
		printk(KERN_DEBUG "field=%s\n",
			prt_names(p->u.vsync.field, v4l2_field_names));
		break;
	case V4L2_EVENT_CTRL:
		c = &p->u.ctrl;
		printk(KERN_DEBUG "changes=0x%x, type=%u, ",
			c->changes, c->type);
		if (c->type == V4L2_CTRL_TYPE_INTEGER64)
			pr_cont("value64=%lld, ", c->value64);
		else
			pr_cont("value=%d, ", c->value);
		pr_cont("flags=0x%x, minimum=%d, maximum=%d, step=%d, default_value=%d\n",
			c->flags, c->minimum, c->maximum,
			c->step, c->default_value);
		break;
	case V4L2_EVENT_FRAME_SYNC:
		pr_cont("frame_sequence=%u\n",
			p->u.frame_sync.frame_sequence);
		break;
	}
}

static void v4l_print_event_subscription(const void *arg, bool write_only)
{
	const struct v4l2_event_subscription *p = arg;

	pr_cont("type=0x%x, id=0x%x, flags=0x%x\n",
			p->type, p->id, p->flags);
}

static void v4l_print_sliced_vbi_cap(const void *arg, bool write_only)
{
	const struct v4l2_sliced_vbi_cap *p = arg;
	int i;

	pr_cont("type=%s, service_set=0x%08x\n",
			prt_names(p->type, v4l2_type_names), p->service_set);
	for (i = 0; i < 24; i++)
		printk(KERN_DEBUG "line[%02u]=0x%04x, 0x%04x\n", i,
				p->service_lines[0][i],
				p->service_lines[1][i]);
}

static void v4l_print_freq_band(const void *arg, bool write_only)
{
	const struct v4l2_frequency_band *p = arg;

	pr_cont("tuner=%u, type=%u, index=%u, capability=0x%x, rangelow=%u, rangehigh=%u, modulation=0x%x\n",
			p->tuner, p->type, p->index,
			p->capability, p->rangelow,
			p->rangehigh, p->modulation);
}

static void v4l_print_edid(const void *arg, bool write_only)
{
	const struct v4l2_edid *p = arg;

	pr_cont("pad=%u, start_block=%u, blocks=%u\n",
		p->pad, p->start_block, p->blocks);
}

static void v4l_print_u32(const void *arg, bool write_only)
{
	pr_cont("value=%u\n", *(const u32 *)arg);
}

static void v4l_print_newline(const void *arg, bool write_only)
{
	pr_cont("\n");
}

static void v4l_print_default(const void *arg, bool write_only)
{
	pr_cont("driver-specific ioctl\n");
}

static bool check_ext_ctrls(struct v4l2_ext_controls *c, unsigned long ioctl)
{
	__u32 i;

	/* zero the reserved fields */
	c->reserved[0] = 0;
	for (i = 0; i < c->count; i++)
		c->controls[i].reserved2[0] = 0;

	switch (c->which) {
	case V4L2_CID_PRIVATE_BASE:
		/*
		 * V4L2_CID_PRIVATE_BASE cannot be used as control class
		 * when using extended controls.
		 * Only when passed in through VIDIOC_G_CTRL and VIDIOC_S_CTRL
		 * is it allowed for backwards compatibility.
		 */
		if (ioctl == VIDIOC_G_CTRL || ioctl == VIDIOC_S_CTRL)
			return false;
		break;
	case V4L2_CTRL_WHICH_DEF_VAL:
		/* Default value cannot be changed */
		if (ioctl == VIDIOC_S_EXT_CTRLS ||
		    ioctl == VIDIOC_TRY_EXT_CTRLS) {
			c->error_idx = c->count;
			return false;
		}
		return true;
	case V4L2_CTRL_WHICH_CUR_VAL:
		return true;
	case V4L2_CTRL_WHICH_REQUEST_VAL:
		c->error_idx = c->count;
		return false;
	}

	/* Check that all controls are from the same control class. */
	for (i = 0; i < c->count; i++) {
		if (V4L2_CTRL_ID2WHICH(c->controls[i].id) != c->which) {
			c->error_idx = ioctl == VIDIOC_TRY_EXT_CTRLS ? i :
								      c->count;
			return false;
		}
	}
	return true;
}

static void v4l_fill_fmtdesc(struct v4l2_fmtdesc *fmt)
{
	const unsigned sz = sizeof(fmt->description);
	const char *descr = NULL;
	u32 flags = 0;

	/*
	 * We depart from the normal coding style here since the descriptions
	 * should be aligned so it is easy to see which descriptions will be
	 * longer than 31 characters (the max length for a description).
	 * And frankly, this is easier to read anyway.
	 *
	 * Note that gcc will use O(log N) comparisons to find the right case.
	 */
	switch (fmt->pixelformat) {
	/* Max description length mask:	descr = "0123456789012345678901234567890" */
	case V4L2_PIX_FMT_RGB332:	descr = "8-bit RGB 3-3-2"; break;
	case V4L2_PIX_FMT_RGB444:	descr = "16-bit A/XRGB 4-4-4-4"; break;
	case V4L2_PIX_FMT_ARGB444:	descr = "16-bit ARGB 4-4-4-4"; break;
	case V4L2_PIX_FMT_XRGB444:	descr = "16-bit XRGB 4-4-4-4"; break;
	case V4L2_PIX_FMT_RGBA444:	descr = "16-bit RGBA 4-4-4-4"; break;
	case V4L2_PIX_FMT_RGBX444:	descr = "16-bit RGBX 4-4-4-4"; break;
	case V4L2_PIX_FMT_ABGR444:	descr = "16-bit ABGR 4-4-4-4"; break;
	case V4L2_PIX_FMT_XBGR444:	descr = "16-bit XBGR 4-4-4-4"; break;
	case V4L2_PIX_FMT_BGRA444:	descr = "16-bit BGRA 4-4-4-4"; break;
	case V4L2_PIX_FMT_BGRX444:	descr = "16-bit BGRX 4-4-4-4"; break;
	case V4L2_PIX_FMT_RGB555:	descr = "16-bit A/XRGB 1-5-5-5"; break;
	case V4L2_PIX_FMT_ARGB555:	descr = "16-bit ARGB 1-5-5-5"; break;
	case V4L2_PIX_FMT_XRGB555:	descr = "16-bit XRGB 1-5-5-5"; break;
	case V4L2_PIX_FMT_ABGR555:	descr = "16-bit ABGR 1-5-5-5"; break;
	case V4L2_PIX_FMT_XBGR555:	descr = "16-bit XBGR 1-5-5-5"; break;
	case V4L2_PIX_FMT_RGBA555:	descr = "16-bit RGBA 5-5-5-1"; break;
	case V4L2_PIX_FMT_RGBX555:	descr = "16-bit RGBX 5-5-5-1"; break;
	case V4L2_PIX_FMT_BGRA555:	descr = "16-bit BGRA 5-5-5-1"; break;
	case V4L2_PIX_FMT_BGRX555:	descr = "16-bit BGRX 5-5-5-1"; break;
	case V4L2_PIX_FMT_RGB565:	descr = "16-bit RGB 5-6-5"; break;
	case V4L2_PIX_FMT_RGB555X:	descr = "16-bit A/XRGB 1-5-5-5 BE"; break;
	case V4L2_PIX_FMT_ARGB555X:	descr = "16-bit ARGB 1-5-5-5 BE"; break;
	case V4L2_PIX_FMT_XRGB555X:	descr = "16-bit XRGB 1-5-5-5 BE"; break;
	case V4L2_PIX_FMT_RGB565X:	descr = "16-bit RGB 5-6-5 BE"; break;
	case V4L2_PIX_FMT_BGR666:	descr = "18-bit BGRX 6-6-6-14"; break;
	case V4L2_PIX_FMT_BGR24:	descr = "24-bit BGR 8-8-8"; break;
	case V4L2_PIX_FMT_RGB24:	descr = "24-bit RGB 8-8-8"; break;
	case V4L2_PIX_FMT_BGR32:	descr = "32-bit BGRA/X 8-8-8-8"; break;
	case V4L2_PIX_FMT_ABGR32:	descr = "32-bit BGRA 8-8-8-8"; break;
	case V4L2_PIX_FMT_XBGR32:	descr = "32-bit BGRX 8-8-8-8"; break;
	case V4L2_PIX_FMT_RGB32:	descr = "32-bit A/XRGB 8-8-8-8"; break;
	case V4L2_PIX_FMT_ARGB32:	descr = "32-bit ARGB 8-8-8-8"; break;
	case V4L2_PIX_FMT_XRGB32:	descr = "32-bit XRGB 8-8-8-8"; break;
	case V4L2_PIX_FMT_BGRA32:	descr = "32-bit ABGR 8-8-8-8"; break;
	case V4L2_PIX_FMT_BGRX32:	descr = "32-bit XBGR 8-8-8-8"; break;
	case V4L2_PIX_FMT_RGBA32:	descr = "32-bit RGBA 8-8-8-8"; break;
	case V4L2_PIX_FMT_RGBX32:	descr = "32-bit RGBX 8-8-8-8"; break;
	case V4L2_PIX_FMT_GREY:		descr = "8-bit Greyscale"; break;
	case V4L2_PIX_FMT_Y4:		descr = "4-bit Greyscale"; break;
	case V4L2_PIX_FMT_Y6:		descr = "6-bit Greyscale"; break;
	case V4L2_PIX_FMT_Y10:		descr = "10-bit Greyscale"; break;
	case V4L2_PIX_FMT_Y12:		descr = "12-bit Greyscale"; break;
	case V4L2_PIX_FMT_Y14:		descr = "14-bit Greyscale"; break;
	case V4L2_PIX_FMT_Y16:		descr = "16-bit Greyscale"; break;
	case V4L2_PIX_FMT_Y16_BE:	descr = "16-bit Greyscale BE"; break;
	case V4L2_PIX_FMT_Y10BPACK:	descr = "10-bit Greyscale (Packed)"; break;
	case V4L2_PIX_FMT_Y10P:		descr = "10-bit Greyscale (MIPI Packed)"; break;
	case V4L2_PIX_FMT_Y8I:		descr = "Interleaved 8-bit Greyscale"; break;
	case V4L2_PIX_FMT_Y12I:		descr = "Interleaved 12-bit Greyscale"; break;
	case V4L2_PIX_FMT_Z16:		descr = "16-bit Depth"; break;
	case V4L2_PIX_FMT_INZI:		descr = "Planar 10:16 Greyscale Depth"; break;
	case V4L2_PIX_FMT_CNF4:		descr = "4-bit Depth Confidence (Packed)"; break;
	case V4L2_PIX_FMT_PAL8:		descr = "8-bit Palette"; break;
	case V4L2_PIX_FMT_UV8:		descr = "8-bit Chrominance UV 4-4"; break;
	case V4L2_PIX_FMT_YVU410:	descr = "Planar YVU 4:1:0"; break;
	case V4L2_PIX_FMT_YVU420:	descr = "Planar YVU 4:2:0"; break;
	case V4L2_PIX_FMT_YUYV:		descr = "YUYV 4:2:2"; break;
	case V4L2_PIX_FMT_YYUV:		descr = "YYUV 4:2:2"; break;
	case V4L2_PIX_FMT_YVYU:		descr = "YVYU 4:2:2"; break;
	case V4L2_PIX_FMT_UYVY:		descr = "UYVY 4:2:2"; break;
	case V4L2_PIX_FMT_VYUY:		descr = "VYUY 4:2:2"; break;
	case V4L2_PIX_FMT_YUV422P:	descr = "Planar YUV 4:2:2"; break;
	case V4L2_PIX_FMT_YUV411P:	descr = "Planar YUV 4:1:1"; break;
	case V4L2_PIX_FMT_Y41P:		descr = "YUV 4:1:1 (Packed)"; break;
	case V4L2_PIX_FMT_YUV444:	descr = "16-bit A/XYUV 4-4-4-4"; break;
	case V4L2_PIX_FMT_YUV555:	descr = "16-bit A/XYUV 1-5-5-5"; break;
	case V4L2_PIX_FMT_YUV565:	descr = "16-bit YUV 5-6-5"; break;
	case V4L2_PIX_FMT_YUV24:	descr = "24-bit YUV 4:4:4 8-8-8"; break;
	case V4L2_PIX_FMT_YUV32:	descr = "32-bit A/XYUV 8-8-8-8"; break;
	case V4L2_PIX_FMT_AYUV32:	descr = "32-bit AYUV 8-8-8-8"; break;
	case V4L2_PIX_FMT_XYUV32:	descr = "32-bit XYUV 8-8-8-8"; break;
	case V4L2_PIX_FMT_VUYA32:	descr = "32-bit VUYA 8-8-8-8"; break;
	case V4L2_PIX_FMT_VUYX32:	descr = "32-bit VUYX 8-8-8-8"; break;
	case V4L2_PIX_FMT_YUV410:	descr = "Planar YUV 4:1:0"; break;
	case V4L2_PIX_FMT_YUV420:	descr = "Planar YUV 4:2:0"; break;
	case V4L2_PIX_FMT_HI240:	descr = "8-bit Dithered RGB (BTTV)"; break;
	case V4L2_PIX_FMT_HM12:		descr = "YUV 4:2:0 (16x16 Macroblocks)"; break;
	case V4L2_PIX_FMT_M420:		descr = "YUV 4:2:0 (M420)"; break;
	case V4L2_PIX_FMT_NV12:		descr = "Y/CbCr 4:2:0"; break;
	case V4L2_PIX_FMT_NV21:		descr = "Y/CrCb 4:2:0"; break;
	case V4L2_PIX_FMT_NV16:		descr = "Y/CbCr 4:2:2"; break;
	case V4L2_PIX_FMT_NV61:		descr = "Y/CrCb 4:2:2"; break;
	case V4L2_PIX_FMT_NV24:		descr = "Y/CbCr 4:4:4"; break;
	case V4L2_PIX_FMT_NV42:		descr = "Y/CrCb 4:4:4"; break;
	case V4L2_PIX_FMT_NV12M:	descr = "Y/CbCr 4:2:0 (N-C)"; break;
	case V4L2_PIX_FMT_NV21M:	descr = "Y/CrCb 4:2:0 (N-C)"; break;
	case V4L2_PIX_FMT_NV16M:	descr = "Y/CbCr 4:2:2 (N-C)"; break;
	case V4L2_PIX_FMT_NV61M:	descr = "Y/CrCb 4:2:2 (N-C)"; break;
	case V4L2_PIX_FMT_NV12MT:	descr = "Y/CbCr 4:2:0 (64x32 MB, N-C)"; break;
	case V4L2_PIX_FMT_NV12MT_16X16:	descr = "Y/CbCr 4:2:0 (16x16 MB, N-C)"; break;
	case V4L2_PIX_FMT_YUV420M:	descr = "Planar YUV 4:2:0 (N-C)"; break;
	case V4L2_PIX_FMT_YVU420M:	descr = "Planar YVU 4:2:0 (N-C)"; break;
	case V4L2_PIX_FMT_YUV422M:	descr = "Planar YUV 4:2:2 (N-C)"; break;
	case V4L2_PIX_FMT_YVU422M:	descr = "Planar YVU 4:2:2 (N-C)"; break;
	case V4L2_PIX_FMT_YUV444M:	descr = "Planar YUV 4:4:4 (N-C)"; break;
	case V4L2_PIX_FMT_YVU444M:	descr = "Planar YVU 4:4:4 (N-C)"; break;
	case V4L2_PIX_FMT_SBGGR8:	descr = "8-bit Bayer BGBG/GRGR"; break;
	case V4L2_PIX_FMT_SGBRG8:	descr = "8-bit Bayer GBGB/RGRG"; break;
	case V4L2_PIX_FMT_SGRBG8:	descr = "8-bit Bayer GRGR/BGBG"; break;
	case V4L2_PIX_FMT_SRGGB8:	descr = "8-bit Bayer RGRG/GBGB"; break;
	case V4L2_PIX_FMT_SBGGR10:	descr = "10-bit Bayer BGBG/GRGR"; break;
	case V4L2_PIX_FMT_SGBRG10:	descr = "10-bit Bayer GBGB/RGRG"; break;
	case V4L2_PIX_FMT_SGRBG10:	descr = "10-bit Bayer GRGR/BGBG"; break;
	case V4L2_PIX_FMT_SRGGB10:	descr = "10-bit Bayer RGRG/GBGB"; break;
	case V4L2_PIX_FMT_SBGGR10P:	descr = "10-bit Bayer BGBG/GRGR Packed"; break;
	case V4L2_PIX_FMT_SGBRG10P:	descr = "10-bit Bayer GBGB/RGRG Packed"; break;
	case V4L2_PIX_FMT_SGRBG10P:	descr = "10-bit Bayer GRGR/BGBG Packed"; break;
	case V4L2_PIX_FMT_SRGGB10P:	descr = "10-bit Bayer RGRG/GBGB Packed"; break;
	case V4L2_PIX_FMT_IPU3_SBGGR10: descr = "10-bit bayer BGGR IPU3 Packed"; break;
	case V4L2_PIX_FMT_IPU3_SGBRG10: descr = "10-bit bayer GBRG IPU3 Packed"; break;
	case V4L2_PIX_FMT_IPU3_SGRBG10: descr = "10-bit bayer GRBG IPU3 Packed"; break;
	case V4L2_PIX_FMT_IPU3_SRGGB10: descr = "10-bit bayer RGGB IPU3 Packed"; break;
	case V4L2_PIX_FMT_SBGGR10ALAW8:	descr = "8-bit Bayer BGBG/GRGR (A-law)"; break;
	case V4L2_PIX_FMT_SGBRG10ALAW8:	descr = "8-bit Bayer GBGB/RGRG (A-law)"; break;
	case V4L2_PIX_FMT_SGRBG10ALAW8:	descr = "8-bit Bayer GRGR/BGBG (A-law)"; break;
	case V4L2_PIX_FMT_SRGGB10ALAW8:	descr = "8-bit Bayer RGRG/GBGB (A-law)"; break;
	case V4L2_PIX_FMT_SBGGR10DPCM8:	descr = "8-bit Bayer BGBG/GRGR (DPCM)"; break;
	case V4L2_PIX_FMT_SGBRG10DPCM8:	descr = "8-bit Bayer GBGB/RGRG (DPCM)"; break;
	case V4L2_PIX_FMT_SGRBG10DPCM8:	descr = "8-bit Bayer GRGR/BGBG (DPCM)"; break;
	case V4L2_PIX_FMT_SRGGB10DPCM8:	descr = "8-bit Bayer RGRG/GBGB (DPCM)"; break;
	case V4L2_PIX_FMT_SBGGR12:	descr = "12-bit Bayer BGBG/GRGR"; break;
	case V4L2_PIX_FMT_SGBRG12:	descr = "12-bit Bayer GBGB/RGRG"; break;
	case V4L2_PIX_FMT_SGRBG12:	descr = "12-bit Bayer GRGR/BGBG"; break;
	case V4L2_PIX_FMT_SRGGB12:	descr = "12-bit Bayer RGRG/GBGB"; break;
	case V4L2_PIX_FMT_SBGGR12P:	descr = "12-bit Bayer BGBG/GRGR Packed"; break;
	case V4L2_PIX_FMT_SGBRG12P:	descr = "12-bit Bayer GBGB/RGRG Packed"; break;
	case V4L2_PIX_FMT_SGRBG12P:	descr = "12-bit Bayer GRGR/BGBG Packed"; break;
	case V4L2_PIX_FMT_SRGGB12P:	descr = "12-bit Bayer RGRG/GBGB Packed"; break;
	case V4L2_PIX_FMT_SBGGR14:	descr = "14-bit Bayer BGBG/GRGR"; break;
	case V4L2_PIX_FMT_SGBRG14:	descr = "14-bit Bayer GBGB/RGRG"; break;
	case V4L2_PIX_FMT_SGRBG14:	descr = "14-bit Bayer GRGR/BGBG"; break;
	case V4L2_PIX_FMT_SRGGB14:	descr = "14-bit Bayer RGRG/GBGB"; break;
	case V4L2_PIX_FMT_SBGGR14P:	descr = "14-bit Bayer BGBG/GRGR Packed"; break;
	case V4L2_PIX_FMT_SGBRG14P:	descr = "14-bit Bayer GBGB/RGRG Packed"; break;
	case V4L2_PIX_FMT_SGRBG14P:	descr = "14-bit Bayer GRGR/BGBG Packed"; break;
	case V4L2_PIX_FMT_SRGGB14P:	descr = "14-bit Bayer RGRG/GBGB Packed"; break;
	case V4L2_PIX_FMT_SBGGR16:	descr = "16-bit Bayer BGBG/GRGR"; break;
	case V4L2_PIX_FMT_SGBRG16:	descr = "16-bit Bayer GBGB/RGRG"; break;
	case V4L2_PIX_FMT_SGRBG16:	descr = "16-bit Bayer GRGR/BGBG"; break;
	case V4L2_PIX_FMT_SRGGB16:	descr = "16-bit Bayer RGRG/GBGB"; break;
	case V4L2_PIX_FMT_SN9C20X_I420:	descr = "GSPCA SN9C20X I420"; break;
	case V4L2_PIX_FMT_SPCA501:	descr = "GSPCA SPCA501"; break;
	case V4L2_PIX_FMT_SPCA505:	descr = "GSPCA SPCA505"; break;
	case V4L2_PIX_FMT_SPCA508:	descr = "GSPCA SPCA508"; break;
	case V4L2_PIX_FMT_STV0680:	descr = "GSPCA STV0680"; break;
	case V4L2_PIX_FMT_TM6000:	descr = "A/V + VBI Mux Packet"; break;
	case V4L2_PIX_FMT_CIT_YYVYUY:	descr = "GSPCA CIT YYVYUY"; break;
	case V4L2_PIX_FMT_KONICA420:	descr = "GSPCA KONICA420"; break;
	case V4L2_PIX_FMT_HSV24:	descr = "24-bit HSV 8-8-8"; break;
	case V4L2_PIX_FMT_HSV32:	descr = "32-bit XHSV 8-8-8-8"; break;
	case V4L2_SDR_FMT_CU8:		descr = "Complex U8"; break;
	case V4L2_SDR_FMT_CU16LE:	descr = "Complex U16LE"; break;
	case V4L2_SDR_FMT_CS8:		descr = "Complex S8"; break;
	case V4L2_SDR_FMT_CS14LE:	descr = "Complex S14LE"; break;
	case V4L2_SDR_FMT_RU12LE:	descr = "Real U12LE"; break;
	case V4L2_SDR_FMT_PCU16BE:	descr = "Planar Complex U16BE"; break;
	case V4L2_SDR_FMT_PCU18BE:	descr = "Planar Complex U18BE"; break;
	case V4L2_SDR_FMT_PCU20BE:	descr = "Planar Complex U20BE"; break;
	case V4L2_TCH_FMT_DELTA_TD16:	descr = "16-bit Signed Deltas"; break;
	case V4L2_TCH_FMT_DELTA_TD08:	descr = "8-bit Signed Deltas"; break;
	case V4L2_TCH_FMT_TU16:		descr = "16-bit Unsigned Touch Data"; break;
	case V4L2_TCH_FMT_TU08:		descr = "8-bit Unsigned Touch Data"; break;
	case V4L2_META_FMT_VSP1_HGO:	descr = "R-Car VSP1 1-D Histogram"; break;
	case V4L2_META_FMT_VSP1_HGT:	descr = "R-Car VSP1 2-D Histogram"; break;
	case V4L2_META_FMT_UVC:		descr = "UVC Payload Header Metadata"; break;
	case V4L2_META_FMT_D4XX:	descr = "Intel D4xx UVC Metadata"; break;
	case V4L2_META_FMT_VIVID:       descr = "Vivid Metadata"; break;
	case V4L2_META_FMT_RK_ISP1_PARAMS:	descr = "Rockchip ISP1 3A Parameters"; break;
	case V4L2_META_FMT_RK_ISP1_STAT_3A:	descr = "Rockchip ISP1 3A Statistics"; break;

	default:
		/* Compressed formats */
		flags = V4L2_FMT_FLAG_COMPRESSED;
		switch (fmt->pixelformat) {
		/* Max description length mask:	descr = "0123456789012345678901234567890" */
		case V4L2_PIX_FMT_MJPEG:	descr = "Motion-JPEG"; break;
		case V4L2_PIX_FMT_JPEG:		descr = "JFIF JPEG"; break;
		case V4L2_PIX_FMT_DV:		descr = "1394"; break;
		case V4L2_PIX_FMT_MPEG:		descr = "MPEG-1/2/4"; break;
		case V4L2_PIX_FMT_H264:		descr = "H.264"; break;
		case V4L2_PIX_FMT_H264_NO_SC:	descr = "H.264 (No Start Codes)"; break;
		case V4L2_PIX_FMT_H264_MVC:	descr = "H.264 MVC"; break;
		case V4L2_PIX_FMT_H264_SLICE:	descr = "H.264 Parsed Slice Data"; break;
		case V4L2_PIX_FMT_H263:		descr = "H.263"; break;
		case V4L2_PIX_FMT_MPEG1:	descr = "MPEG-1 ES"; break;
		case V4L2_PIX_FMT_MPEG2:	descr = "MPEG-2 ES"; break;
		case V4L2_PIX_FMT_MPEG2_SLICE:	descr = "MPEG-2 Parsed Slice Data"; break;
		case V4L2_PIX_FMT_MPEG4:	descr = "MPEG-4 Part 2 ES"; break;
		case V4L2_PIX_FMT_XVID:		descr = "Xvid"; break;
		case V4L2_PIX_FMT_VC1_ANNEX_G:	descr = "VC-1 (SMPTE 412M Annex G)"; break;
		case V4L2_PIX_FMT_VC1_ANNEX_L:	descr = "VC-1 (SMPTE 412M Annex L)"; break;
		case V4L2_PIX_FMT_VP8:		descr = "VP8"; break;
		case V4L2_PIX_FMT_VP8_FRAME:    descr = "VP8 Frame"; break;
		case V4L2_PIX_FMT_VP9:		descr = "VP9"; break;
		case V4L2_PIX_FMT_HEVC:		descr = "HEVC"; break; /* aka H.265 */
		case V4L2_PIX_FMT_HEVC_SLICE:	descr = "HEVC Parsed Slice Data"; break;
		case V4L2_PIX_FMT_FWHT:		descr = "FWHT"; break; /* used in vicodec */
		case V4L2_PIX_FMT_FWHT_STATELESS:	descr = "FWHT Stateless"; break; /* used in vicodec */
		case V4L2_PIX_FMT_CPIA1:	descr = "GSPCA CPiA YUV"; break;
		case V4L2_PIX_FMT_WNVA:		descr = "WNVA"; break;
		case V4L2_PIX_FMT_SN9C10X:	descr = "GSPCA SN9C10X"; break;
		case V4L2_PIX_FMT_PWC1:		descr = "Raw Philips Webcam Type (Old)"; break;
		case V4L2_PIX_FMT_PWC2:		descr = "Raw Philips Webcam Type (New)"; break;
		case V4L2_PIX_FMT_ET61X251:	descr = "GSPCA ET61X251"; break;
		case V4L2_PIX_FMT_SPCA561:	descr = "GSPCA SPCA561"; break;
		case V4L2_PIX_FMT_PAC207:	descr = "GSPCA PAC207"; break;
		case V4L2_PIX_FMT_MR97310A:	descr = "GSPCA MR97310A"; break;
		case V4L2_PIX_FMT_JL2005BCD:	descr = "GSPCA JL2005BCD"; break;
		case V4L2_PIX_FMT_SN9C2028:	descr = "GSPCA SN9C2028"; break;
		case V4L2_PIX_FMT_SQ905C:	descr = "GSPCA SQ905C"; break;
		case V4L2_PIX_FMT_PJPG:		descr = "GSPCA PJPG"; break;
		case V4L2_PIX_FMT_OV511:	descr = "GSPCA OV511"; break;
		case V4L2_PIX_FMT_OV518:	descr = "GSPCA OV518"; break;
		case V4L2_PIX_FMT_JPGL:		descr = "JPEG Lite"; break;
		case V4L2_PIX_FMT_SE401:	descr = "GSPCA SE401"; break;
		case V4L2_PIX_FMT_S5C_UYVY_JPG:	descr = "S5C73MX interleaved UYVY/JPEG"; break;
		case V4L2_PIX_FMT_MT21C:	descr = "Mediatek Compressed Format"; break;
		case V4L2_PIX_FMT_SUNXI_TILED_NV12: descr = "Sunxi Tiled NV12 Format"; break;
		default:
			if (fmt->description[0])
				return;
			WARN(1, "Unknown pixelformat 0x%08x\n", fmt->pixelformat);
			flags = 0;
			snprintf(fmt->description, sz, "%p4cc",
				 &fmt->pixelformat);
			break;
		}
	}

	if (descr)
		WARN_ON(strscpy(fmt->description, descr, sz) < 0);
	fmt->flags |= flags;
}


static int v4l_enum_fmt(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_fmtdesc *p = arg;
	int ret = check_fmt(file, p->type);
	u32 mbus_code;
	u32 cap_mask;

	if (ret)
		return ret;
	ret = -EINVAL;

	if (!(vdev->device_caps & V4L2_CAP_IO_MC))
		p->mbus_code = 0;

	mbus_code = p->mbus_code;
	CLEAR_AFTER_FIELD(p, type);
	p->mbus_code = mbus_code;

	switch (p->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		cap_mask = V4L2_CAP_VIDEO_CAPTURE_MPLANE |
			   V4L2_CAP_VIDEO_M2M_MPLANE;
		if (!!(vdev->device_caps & cap_mask) !=
		    (p->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE))
			break;

		if (unlikely(!ops->vidioc_enum_fmt_vid_cap))
			break;
		ret = ops->vidioc_enum_fmt_vid_cap(file, fh, arg);
		break;
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
		if (unlikely(!ops->vidioc_enum_fmt_vid_overlay))
			break;
		ret = ops->vidioc_enum_fmt_vid_overlay(file, fh, arg);
		break;
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		cap_mask = V4L2_CAP_VIDEO_OUTPUT_MPLANE |
			   V4L2_CAP_VIDEO_M2M_MPLANE;
		if (!!(vdev->device_caps & cap_mask) !=
		    (p->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE))
			break;

		if (unlikely(!ops->vidioc_enum_fmt_vid_out))
			break;
		ret = ops->vidioc_enum_fmt_vid_out(file, fh, arg);
		break;
	case V4L2_BUF_TYPE_SDR_CAPTURE:
		if (unlikely(!ops->vidioc_enum_fmt_sdr_cap))
			break;
		ret = ops->vidioc_enum_fmt_sdr_cap(file, fh, arg);
		break;
	case V4L2_BUF_TYPE_SDR_OUTPUT:
		if (unlikely(!ops->vidioc_enum_fmt_sdr_out))
			break;
		ret = ops->vidioc_enum_fmt_sdr_out(file, fh, arg);
		break;
	case V4L2_BUF_TYPE_META_CAPTURE:
		if (unlikely(!ops->vidioc_enum_fmt_meta_cap))
			break;
		ret = ops->vidioc_enum_fmt_meta_cap(file, fh, arg);
		break;
	case V4L2_BUF_TYPE_META_OUTPUT:
		if (unlikely(!ops->vidioc_enum_fmt_meta_out))
			break;
		ret = ops->vidioc_enum_fmt_meta_out(file, fh, arg);
		break;
	}
	if (ret == 0)
		v4l_fill_fmtdesc(p);
	return ret;
}

static void v4l_pix_format_touch(struct v4l2_pix_format *p)
{
	/*
	 * The v4l2_pix_format structure contains fields that make no sense for
	 * touch. Set them to default values in this case.
	 */

	p->field = V4L2_FIELD_NONE;
	p->colorspace = V4L2_COLORSPACE_RAW;
	p->flags = 0;
	p->ycbcr_enc = 0;
	p->quantization = 0;
	p->xfer_func = 0;
}

static int v4l_g_fmt(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct v4l2_format *p = arg;
	struct video_device *vfd = video_devdata(file);
	int ret = check_fmt(file, p->type);

	if (ret)
		return ret;

	/*
	 * fmt can't be cleared for these overlay types due to the 'clips'
	 * 'clipcount' and 'bitmap' pointers in struct v4l2_window.
	 * Those are provided by the user. So handle these two overlay types
	 * first, and then just do a simple memset for the other types.
	 */
	switch (p->type) {
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY: {
		struct v4l2_clip *clips = p->fmt.win.clips;
		u32 clipcount = p->fmt.win.clipcount;
		void __user *bitmap = p->fmt.win.bitmap;

		memset(&p->fmt, 0, sizeof(p->fmt));
		p->fmt.win.clips = clips;
		p->fmt.win.clipcount = clipcount;
		p->fmt.win.bitmap = bitmap;
		break;
	}
	default:
		memset(&p->fmt, 0, sizeof(p->fmt));
		break;
	}

	switch (p->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		if (unlikely(!ops->vidioc_g_fmt_vid_cap))
			break;
		p->fmt.pix.priv = V4L2_PIX_FMT_PRIV_MAGIC;
		ret = ops->vidioc_g_fmt_vid_cap(file, fh, arg);
		/* just in case the driver zeroed it again */
		p->fmt.pix.priv = V4L2_PIX_FMT_PRIV_MAGIC;
		if (vfd->vfl_type == VFL_TYPE_TOUCH)
			v4l_pix_format_touch(&p->fmt.pix);
		return ret;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		return ops->vidioc_g_fmt_vid_cap_mplane(file, fh, arg);
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
		return ops->vidioc_g_fmt_vid_overlay(file, fh, arg);
	case V4L2_BUF_TYPE_VBI_CAPTURE:
		return ops->vidioc_g_fmt_vbi_cap(file, fh, arg);
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
		return ops->vidioc_g_fmt_sliced_vbi_cap(file, fh, arg);
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		if (unlikely(!ops->vidioc_g_fmt_vid_out))
			break;
		p->fmt.pix.priv = V4L2_PIX_FMT_PRIV_MAGIC;
		ret = ops->vidioc_g_fmt_vid_out(file, fh, arg);
		/* just in case the driver zeroed it again */
		p->fmt.pix.priv = V4L2_PIX_FMT_PRIV_MAGIC;
		return ret;
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		return ops->vidioc_g_fmt_vid_out_mplane(file, fh, arg);
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
		return ops->vidioc_g_fmt_vid_out_overlay(file, fh, arg);
	case V4L2_BUF_TYPE_VBI_OUTPUT:
		return ops->vidioc_g_fmt_vbi_out(file, fh, arg);
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		return ops->vidioc_g_fmt_sliced_vbi_out(file, fh, arg);
	case V4L2_BUF_TYPE_SDR_CAPTURE:
		return ops->vidioc_g_fmt_sdr_cap(file, fh, arg);
	case V4L2_BUF_TYPE_SDR_OUTPUT:
		return ops->vidioc_g_fmt_sdr_out(file, fh, arg);
	case V4L2_BUF_TYPE_META_CAPTURE:
		return ops->vidioc_g_fmt_meta_cap(file, fh, arg);
	case V4L2_BUF_TYPE_META_OUTPUT:
		return ops->vidioc_g_fmt_meta_out(file, fh, arg);
	}
	return -EINVAL;
}


static void v4l_sanitize_format(struct v4l2_format *fmt)
{
	unsigned int offset;

	/* Make sure num_planes is not bogus */
	if (fmt->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE ||
	    fmt->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
		fmt->fmt.pix_mp.num_planes = min_t(u32, fmt->fmt.pix_mp.num_planes,
					       VIDEO_MAX_PLANES);

	/*
	 * The v4l2_pix_format structure has been extended with fields that were
	 * not previously required to be set to zero by applications. The priv
	 * field, when set to a magic value, indicates the the extended fields
	 * are valid. Otherwise they will contain undefined values. To simplify
	 * the API towards drivers zero the extended fields and set the priv
	 * field to the magic value when the extended pixel format structure
	 * isn't used by applications.
	 */

	if (fmt->type != V4L2_BUF_TYPE_VIDEO_CAPTURE &&
	    fmt->type != V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return;

	if (fmt->fmt.pix.priv == V4L2_PIX_FMT_PRIV_MAGIC)
		return;

	fmt->fmt.pix.priv = V4L2_PIX_FMT_PRIV_MAGIC;

	offset = offsetof(struct v4l2_pix_format, priv)
	       + sizeof(fmt->fmt.pix.priv);
	memset(((void *)&fmt->fmt.pix) + offset, 0,
	       sizeof(fmt->fmt.pix) - offset);
}

static int v4l_s_fmt(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct v4l2_format *p = arg;
	struct video_device *vfd = video_devdata(file);
	int ret = check_fmt(file, p->type);
	unsigned int i;

	if (ret)
		return ret;

	ret = v4l_enable_media_source(vfd);
	if (ret)
		return ret;
	v4l_sanitize_format(p);

	switch (p->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		if (unlikely(!ops->vidioc_s_fmt_vid_cap))
			break;
		CLEAR_AFTER_FIELD(p, fmt.pix);
		ret = ops->vidioc_s_fmt_vid_cap(file, fh, arg);
		/* just in case the driver zeroed it again */
		p->fmt.pix.priv = V4L2_PIX_FMT_PRIV_MAGIC;
		if (vfd->vfl_type == VFL_TYPE_TOUCH)
			v4l_pix_format_touch(&p->fmt.pix);
		return ret;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		if (unlikely(!ops->vidioc_s_fmt_vid_cap_mplane))
			break;
		CLEAR_AFTER_FIELD(p, fmt.pix_mp.xfer_func);
		for (i = 0; i < p->fmt.pix_mp.num_planes; i++)
			CLEAR_AFTER_FIELD(&p->fmt.pix_mp.plane_fmt[i],
					  bytesperline);
		return ops->vidioc_s_fmt_vid_cap_mplane(file, fh, arg);
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
		if (unlikely(!ops->vidioc_s_fmt_vid_overlay))
			break;
		CLEAR_AFTER_FIELD(p, fmt.win);
		return ops->vidioc_s_fmt_vid_overlay(file, fh, arg);
	case V4L2_BUF_TYPE_VBI_CAPTURE:
		if (unlikely(!ops->vidioc_s_fmt_vbi_cap))
			break;
		CLEAR_AFTER_FIELD(p, fmt.vbi.flags);
		return ops->vidioc_s_fmt_vbi_cap(file, fh, arg);
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
		if (unlikely(!ops->vidioc_s_fmt_sliced_vbi_cap))
			break;
		CLEAR_AFTER_FIELD(p, fmt.sliced.io_size);
		return ops->vidioc_s_fmt_sliced_vbi_cap(file, fh, arg);
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		if (unlikely(!ops->vidioc_s_fmt_vid_out))
			break;
		CLEAR_AFTER_FIELD(p, fmt.pix);
		ret = ops->vidioc_s_fmt_vid_out(file, fh, arg);
		/* just in case the driver zeroed it again */
		p->fmt.pix.priv = V4L2_PIX_FMT_PRIV_MAGIC;
		return ret;
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		if (unlikely(!ops->vidioc_s_fmt_vid_out_mplane))
			break;
		CLEAR_AFTER_FIELD(p, fmt.pix_mp.xfer_func);
		for (i = 0; i < p->fmt.pix_mp.num_planes; i++)
			CLEAR_AFTER_FIELD(&p->fmt.pix_mp.plane_fmt[i],
					  bytesperline);
		return ops->vidioc_s_fmt_vid_out_mplane(file, fh, arg);
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
		if (unlikely(!ops->vidioc_s_fmt_vid_out_overlay))
			break;
		CLEAR_AFTER_FIELD(p, fmt.win);
		return ops->vidioc_s_fmt_vid_out_overlay(file, fh, arg);
	case V4L2_BUF_TYPE_VBI_OUTPUT:
		if (unlikely(!ops->vidioc_s_fmt_vbi_out))
			break;
		CLEAR_AFTER_FIELD(p, fmt.vbi.flags);
		return ops->vidioc_s_fmt_vbi_out(file, fh, arg);
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		if (unlikely(!ops->vidioc_s_fmt_sliced_vbi_out))
			break;
		CLEAR_AFTER_FIELD(p, fmt.sliced.io_size);
		return ops->vidioc_s_fmt_sliced_vbi_out(file, fh, arg);
	case V4L2_BUF_TYPE_SDR_CAPTURE:
		if (unlikely(!ops->vidioc_s_fmt_sdr_cap))
			break;
		CLEAR_AFTER_FIELD(p, fmt.sdr.buffersize);
		return ops->vidioc_s_fmt_sdr_cap(file, fh, arg);
	case V4L2_BUF_TYPE_SDR_OUTPUT:
		if (unlikely(!ops->vidioc_s_fmt_sdr_out))
			break;
		CLEAR_AFTER_FIELD(p, fmt.sdr.buffersize);
		return ops->vidioc_s_fmt_sdr_out(file, fh, arg);
	case V4L2_BUF_TYPE_META_CAPTURE:
		if (unlikely(!ops->vidioc_s_fmt_meta_cap))
			break;
		CLEAR_AFTER_FIELD(p, fmt.meta);
		return ops->vidioc_s_fmt_meta_cap(file, fh, arg);
	case V4L2_BUF_TYPE_META_OUTPUT:
		if (unlikely(!ops->vidioc_s_fmt_meta_out))
			break;
		CLEAR_AFTER_FIELD(p, fmt.meta);
		return ops->vidioc_s_fmt_meta_out(file, fh, arg);
	}
	return -EINVAL;
}

static int v4l_try_fmt(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct v4l2_format *p = arg;
	struct video_device *vfd = video_devdata(file);
	int ret = check_fmt(file, p->type);
	unsigned int i;

	if (ret)
		return ret;

	v4l_sanitize_format(p);

	switch (p->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		if (unlikely(!ops->vidioc_try_fmt_vid_cap))
			break;
		CLEAR_AFTER_FIELD(p, fmt.pix);
		ret = ops->vidioc_try_fmt_vid_cap(file, fh, arg);
		/* just in case the driver zeroed it again */
		p->fmt.pix.priv = V4L2_PIX_FMT_PRIV_MAGIC;
		if (vfd->vfl_type == VFL_TYPE_TOUCH)
			v4l_pix_format_touch(&p->fmt.pix);
		return ret;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		if (unlikely(!ops->vidioc_try_fmt_vid_cap_mplane))
			break;
		CLEAR_AFTER_FIELD(p, fmt.pix_mp.xfer_func);
		for (i = 0; i < p->fmt.pix_mp.num_planes; i++)
			CLEAR_AFTER_FIELD(&p->fmt.pix_mp.plane_fmt[i],
					  bytesperline);
		return ops->vidioc_try_fmt_vid_cap_mplane(file, fh, arg);
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
		if (unlikely(!ops->vidioc_try_fmt_vid_overlay))
			break;
		CLEAR_AFTER_FIELD(p, fmt.win);
		return ops->vidioc_try_fmt_vid_overlay(file, fh, arg);
	case V4L2_BUF_TYPE_VBI_CAPTURE:
		if (unlikely(!ops->vidioc_try_fmt_vbi_cap))
			break;
		CLEAR_AFTER_FIELD(p, fmt.vbi.flags);
		return ops->vidioc_try_fmt_vbi_cap(file, fh, arg);
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
		if (unlikely(!ops->vidioc_try_fmt_sliced_vbi_cap))
			break;
		CLEAR_AFTER_FIELD(p, fmt.sliced.io_size);
		return ops->vidioc_try_fmt_sliced_vbi_cap(file, fh, arg);
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		if (unlikely(!ops->vidioc_try_fmt_vid_out))
			break;
		CLEAR_AFTER_FIELD(p, fmt.pix);
		ret = ops->vidioc_try_fmt_vid_out(file, fh, arg);
		/* just in case the driver zeroed it again */
		p->fmt.pix.priv = V4L2_PIX_FMT_PRIV_MAGIC;
		return ret;
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		if (unlikely(!ops->vidioc_try_fmt_vid_out_mplane))
			break;
		CLEAR_AFTER_FIELD(p, fmt.pix_mp.xfer_func);
		for (i = 0; i < p->fmt.pix_mp.num_planes; i++)
			CLEAR_AFTER_FIELD(&p->fmt.pix_mp.plane_fmt[i],
					  bytesperline);
		return ops->vidioc_try_fmt_vid_out_mplane(file, fh, arg);
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
		if (unlikely(!ops->vidioc_try_fmt_vid_out_overlay))
			break;
		CLEAR_AFTER_FIELD(p, fmt.win);
		return ops->vidioc_try_fmt_vid_out_overlay(file, fh, arg);
	case V4L2_BUF_TYPE_VBI_OUTPUT:
		if (unlikely(!ops->vidioc_try_fmt_vbi_out))
			break;
		CLEAR_AFTER_FIELD(p, fmt.vbi.flags);
		return ops->vidioc_try_fmt_vbi_out(file, fh, arg);
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		if (unlikely(!ops->vidioc_try_fmt_sliced_vbi_out))
			break;
		CLEAR_AFTER_FIELD(p, fmt.sliced.io_size);
		return ops->vidioc_try_fmt_sliced_vbi_out(file, fh, arg);
	case V4L2_BUF_TYPE_SDR_CAPTURE:
		if (unlikely(!ops->vidioc_try_fmt_sdr_cap))
			break;
		CLEAR_AFTER_FIELD(p, fmt.sdr.buffersize);
		return ops->vidioc_try_fmt_sdr_cap(file, fh, arg);
	case V4L2_BUF_TYPE_SDR_OUTPUT:
		if (unlikely(!ops->vidioc_try_fmt_sdr_out))
			break;
		CLEAR_AFTER_FIELD(p, fmt.sdr.buffersize);
		return ops->vidioc_try_fmt_sdr_out(file, fh, arg);
	case V4L2_BUF_TYPE_META_CAPTURE:
		if (unlikely(!ops->vidioc_try_fmt_meta_cap))
			break;
		CLEAR_AFTER_FIELD(p, fmt.meta);
		return ops->vidioc_try_fmt_meta_cap(file, fh, arg);
	case V4L2_BUF_TYPE_META_OUTPUT:
		if (unlikely(!ops->vidioc_try_fmt_meta_out))
			break;
		CLEAR_AFTER_FIELD(p, fmt.meta);
		return ops->vidioc_try_fmt_meta_out(file, fh, arg);
	}
	return -EINVAL;
}

static int v4l_streamon(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	return ops->vidioc_streamon(file, fh, *(unsigned int *)arg);
}

static int v4l_streamoff(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	return ops->vidioc_streamoff(file, fh, *(unsigned int *)arg);
}

static int v4l_g_tuner(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_tuner *p = arg;
	int err;

	p->type = (vfd->vfl_type == VFL_TYPE_RADIO) ?
			V4L2_TUNER_RADIO : V4L2_TUNER_ANALOG_TV;
	err = ops->vidioc_g_tuner(file, fh, p);
	if (!err)
		p->capability |= V4L2_TUNER_CAP_FREQ_BANDS;
	return err;
}

static int v4l_s_tuner(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_tuner *p = arg;
	int ret;

	ret = v4l_enable_media_source(vfd);
	if (ret)
		return ret;
	p->type = (vfd->vfl_type == VFL_TYPE_RADIO) ?
			V4L2_TUNER_RADIO : V4L2_TUNER_ANALOG_TV;
	return ops->vidioc_s_tuner(file, fh, p);
}

static int v4l_g_modulator(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_modulator *p = arg;
	int err;

	if (vfd->vfl_type == VFL_TYPE_RADIO)
		p->type = V4L2_TUNER_RADIO;

	err = ops->vidioc_g_modulator(file, fh, p);
	if (!err)
		p->capability |= V4L2_TUNER_CAP_FREQ_BANDS;
	return err;
}

static int v4l_s_modulator(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_modulator *p = arg;

	if (vfd->vfl_type == VFL_TYPE_RADIO)
		p->type = V4L2_TUNER_RADIO;

	return ops->vidioc_s_modulator(file, fh, p);
}

static int v4l_g_frequency(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_frequency *p = arg;

	if (vfd->vfl_type == VFL_TYPE_SDR)
		p->type = V4L2_TUNER_SDR;
	else
		p->type = (vfd->vfl_type == VFL_TYPE_RADIO) ?
				V4L2_TUNER_RADIO : V4L2_TUNER_ANALOG_TV;
	return ops->vidioc_g_frequency(file, fh, p);
}

static int v4l_s_frequency(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	const struct v4l2_frequency *p = arg;
	enum v4l2_tuner_type type;
	int ret;

	ret = v4l_enable_media_source(vfd);
	if (ret)
		return ret;
	if (vfd->vfl_type == VFL_TYPE_SDR) {
		if (p->type != V4L2_TUNER_SDR && p->type != V4L2_TUNER_RF)
			return -EINVAL;
	} else {
		type = (vfd->vfl_type == VFL_TYPE_RADIO) ?
				V4L2_TUNER_RADIO : V4L2_TUNER_ANALOG_TV;
		if (type != p->type)
			return -EINVAL;
	}
	return ops->vidioc_s_frequency(file, fh, p);
}


struct std_descr {
	v4l2_std_id std;
	const char *descr;
};

static const struct std_descr standards[] = {
	{ V4L2_STD_NTSC,	"NTSC"      },
	{ V4L2_STD_NTSC_M,	"NTSC-M"    },
	{ V4L2_STD_NTSC_M_JP,	"NTSC-M-JP" },
	{ V4L2_STD_NTSC_M_KR,	"NTSC-M-KR" },
	{ V4L2_STD_NTSC_443,	"NTSC-443"  },
	{ V4L2_STD_PAL,		"PAL"       },
	{ V4L2_STD_PAL_BG,	"PAL-BG"    },
	{ V4L2_STD_PAL_B,	"PAL-B"     },
	{ V4L2_STD_PAL_B1,	"PAL-B1"    },
	{ V4L2_STD_PAL_G,	"PAL-G"     },
	{ V4L2_STD_PAL_H,	"PAL-H"     },
	{ V4L2_STD_PAL_I,	"PAL-I"     },
	{ V4L2_STD_PAL_DK,	"PAL-DK"    },
	{ V4L2_STD_PAL_D,	"PAL-D"     },
	{ V4L2_STD_PAL_D1,	"PAL-D1"    },
	{ V4L2_STD_PAL_K,	"PAL-K"     },
	{ V4L2_STD_PAL_M,	"PAL-M"     },
	{ V4L2_STD_PAL_N,	"PAL-N"     },
	{ V4L2_STD_PAL_Nc,	"PAL-Nc"    },
	{ V4L2_STD_PAL_60,	"PAL-60"    },
	{ V4L2_STD_SECAM,	"SECAM"     },
	{ V4L2_STD_SECAM_B,	"SECAM-B"   },
	{ V4L2_STD_SECAM_G,	"SECAM-G"   },
	{ V4L2_STD_SECAM_H,	"SECAM-H"   },
	{ V4L2_STD_SECAM_DK,	"SECAM-DK"  },
	{ V4L2_STD_SECAM_D,	"SECAM-D"   },
	{ V4L2_STD_SECAM_K,	"SECAM-K"   },
	{ V4L2_STD_SECAM_K1,	"SECAM-K1"  },
	{ V4L2_STD_SECAM_L,	"SECAM-L"   },
	{ V4L2_STD_SECAM_LC,	"SECAM-Lc"  },
	{ 0,			"Unknown"   }
};


int v4l_video_std_enumstd(struct v4l2_standard *vs, v4l2_std_id id)
{
	v4l2_std_id curr_id = 0;
	unsigned int index = vs->index, i, j = 0;
	const char *descr = "";

	/* Return -ENODATA if the id for the current input
	   or output is 0, meaning that it doesn't support this API. */
	if (id == 0)
		return -ENODATA;

	/* Return norm array in a canonical way */
	for (i = 0; i <= index && id; i++) {
		/* last std value in the standards array is 0, so this
		   while always ends there since (id & 0) == 0. */
		while ((id & standards[j].std) != standards[j].std)
			j++;
		curr_id = standards[j].std;
		descr = standards[j].descr;
		j++;
		if (curr_id == 0)
			break;
		if (curr_id != V4L2_STD_PAL &&
				curr_id != V4L2_STD_SECAM &&
				curr_id != V4L2_STD_NTSC)
			id &= ~curr_id;
	}
	if (i <= index)
		return -EINVAL;

	v4l2_video_std_construct(vs, curr_id, descr);
	return 0;
}




static int v4l_enumstd(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_standard *p = arg;

	return v4l_video_std_enumstd(p, vfd->tvnorms);
}

static int v4l_s_std(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	v4l2_std_id id = *(v4l2_std_id *)arg, norm;
	int ret;

	ret = v4l_enable_media_source(vfd);
	if (ret)
		return ret;
	norm = id & vfd->tvnorms;
	if (vfd->tvnorms && !norm)	/* Check if std is supported */
		return -EINVAL;

	/* Calls the specific handler */
	return ops->vidioc_s_std(file, fh, norm);
}

static int v4l_querystd(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	v4l2_std_id *p = arg;
	int ret;

	ret = v4l_enable_media_source(vfd);
	if (ret)
		return ret;
	/*
	 * If no signal is detected, then the driver should return
	 * V4L2_STD_UNKNOWN. Otherwise it should return tvnorms with
	 * any standards that do not apply removed.
	 *
	 * This means that tuners, audio and video decoders can join
	 * their efforts to improve the standards detection.
	 */
	*p = vfd->tvnorms;
	return ops->vidioc_querystd(file, fh, arg);
}

static int v4l_s_hw_freq_seek(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_hw_freq_seek *p = arg;
	enum v4l2_tuner_type type;
	int ret;

	ret = v4l_enable_media_source(vfd);
	if (ret)
		return ret;
	/* s_hw_freq_seek is not supported for SDR for now */
	if (vfd->vfl_type == VFL_TYPE_SDR)
		return -EINVAL;

	type = (vfd->vfl_type == VFL_TYPE_RADIO) ?
		V4L2_TUNER_RADIO : V4L2_TUNER_ANALOG_TV;
	if (p->type != type)
		return -EINVAL;
	return ops->vidioc_s_hw_freq_seek(file, fh, p);
}

static int v4l_overlay(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	return ops->vidioc_overlay(file, fh, *(unsigned int *)arg);
}


static int v4l_reqbufs(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct v4l2_requestbuffers *p = arg;
	printk(KERN_ALERT "v4l_reqbufs : enter!\r\n");
	int ret = check_fmt(file, p->type);
	printk(KERN_ALERT "check_fmt ret = %d\r\n" , ret);
	if (ret)
		return ret;

	CLEAR_AFTER_FIELD(p, capabilities);

	printk(KERN_ALERT "count = %d , mem = %d , type = %d\r\n" , p->count , p->memory , p->type);
	return ops->vidioc_reqbufs(file, fh, p);
}



static int v4l_querybuf(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct v4l2_buffer *p = arg;
	int ret = check_fmt(file, p->type);

	return ret ? ret : ops->vidioc_querybuf(file, fh, p);
}

static int v4l_qbuf(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct v4l2_buffer *p = arg;
	int ret = check_fmt(file, p->type);

	return ret ? ret : ops->vidioc_qbuf(file, fh, p);
}

static int v4l_dqbuf(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct v4l2_buffer *p = arg;
	int ret = check_fmt(file, p->type);

	return ret ? ret : ops->vidioc_dqbuf(file, fh, p);
}

static int v4l_create_bufs(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct v4l2_create_buffers *create = arg;
	int ret = check_fmt(file, create->format.type);

	if (ret)
		return ret;

	CLEAR_AFTER_FIELD(create, capabilities);

	v4l_sanitize_format(&create->format);

	ret = ops->vidioc_create_bufs(file, fh, create);

	if (create->format.type == V4L2_BUF_TYPE_VIDEO_CAPTURE ||
	    create->format.type == V4L2_BUF_TYPE_VIDEO_OUTPUT)
		create->format.fmt.pix.priv = V4L2_PIX_FMT_PRIV_MAGIC;

	return ret;
}

static int v4l_prepare_buf(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct v4l2_buffer *b = arg;
	int ret = check_fmt(file, b->type);

	return ret ? ret : ops->vidioc_prepare_buf(file, fh, b);
}

static int v4l_g_parm(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_streamparm *p = arg;
	v4l2_std_id std;
	int ret = check_fmt(file, p->type);

	if (ret)
		return ret;
	if (ops->vidioc_g_parm)
		return ops->vidioc_g_parm(file, fh, p);
	if (p->type != V4L2_BUF_TYPE_VIDEO_CAPTURE &&
	    p->type != V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
		return -EINVAL;
	if (vfd->device_caps & V4L2_CAP_READWRITE)
		p->parm.capture.readbuffers = 2;
	ret = ops->vidioc_g_std(file, fh, &std);
	if (ret == 0)
		v4l2_video_std_frame_period(std, &p->parm.capture.timeperframe);
	return ret;
}

static int v4l_s_parm(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct v4l2_streamparm *p = arg;
	int ret = check_fmt(file, p->type);

	if (ret)
		return ret;

	/* Note: extendedmode is never used in drivers */
	if (V4L2_TYPE_IS_OUTPUT(p->type)) {
		memset(p->parm.output.reserved, 0,
		       sizeof(p->parm.output.reserved));
		p->parm.output.extendedmode = 0;
		p->parm.output.outputmode &= V4L2_MODE_HIGHQUALITY;
	} else {
		memset(p->parm.capture.reserved, 0,
		       sizeof(p->parm.capture.reserved));
		p->parm.capture.extendedmode = 0;
		p->parm.capture.capturemode &= V4L2_MODE_HIGHQUALITY;
	}
	return ops->vidioc_s_parm(file, fh, p);
}

static int v4l_queryctrl(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_queryctrl *p = arg;
	struct v4l2_fh *vfh =
		test_bit(V4L2_FL_USES_V4L2_FH, &vfd->flags) ? fh : NULL;

	if (vfh && vfh->ctrl_handler)
		return v4l2_queryctrl(vfh->ctrl_handler, p);
	if (vfd->ctrl_handler)
		return v4l2_queryctrl(vfd->ctrl_handler, p);
	if (ops->vidioc_queryctrl)
		return ops->vidioc_queryctrl(file, fh, p);
	return -ENOTTY;
}

static int v4l_query_ext_ctrl(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_query_ext_ctrl *p = arg;
	struct v4l2_fh *vfh =
		test_bit(V4L2_FL_USES_V4L2_FH, &vfd->flags) ? fh : NULL;

	if (vfh && vfh->ctrl_handler)
		return v4l2_query_ext_ctrl(vfh->ctrl_handler, p);
	if (vfd->ctrl_handler)
		return v4l2_query_ext_ctrl(vfd->ctrl_handler, p);
	if (ops->vidioc_query_ext_ctrl)
		return ops->vidioc_query_ext_ctrl(file, fh, p);
	return -ENOTTY;
}

static int v4l_querymenu(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_querymenu *p = arg;
	struct v4l2_fh *vfh =
		test_bit(V4L2_FL_USES_V4L2_FH, &vfd->flags) ? fh : NULL;

	if (vfh && vfh->ctrl_handler)
		return v4l2_querymenu(vfh->ctrl_handler, p);
	if (vfd->ctrl_handler)
		return v4l2_querymenu(vfd->ctrl_handler, p);
	if (ops->vidioc_querymenu)
		return ops->vidioc_querymenu(file, fh, p);
	return -ENOTTY;
}

static int v4l_g_ctrl(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_control *p = arg;
	struct v4l2_fh *vfh =
		test_bit(V4L2_FL_USES_V4L2_FH, &vfd->flags) ? fh : NULL;
	struct v4l2_ext_controls ctrls;
	struct v4l2_ext_control ctrl;

	if (vfh && vfh->ctrl_handler)
		return v4l2_g_ctrl(vfh->ctrl_handler, p);
	if (vfd->ctrl_handler)
		return v4l2_g_ctrl(vfd->ctrl_handler, p);
	if (ops->vidioc_g_ctrl)
		return ops->vidioc_g_ctrl(file, fh, p);
	if (ops->vidioc_g_ext_ctrls == NULL)
		return -ENOTTY;

	ctrls.which = V4L2_CTRL_ID2WHICH(p->id);
	ctrls.count = 1;
	ctrls.controls = &ctrl;
	ctrl.id = p->id;
	ctrl.value = p->value;
	if (check_ext_ctrls(&ctrls, VIDIOC_G_CTRL)) {
		int ret = ops->vidioc_g_ext_ctrls(file, fh, &ctrls);

		if (ret == 0)
			p->value = ctrl.value;
		return ret;
	}
	return -EINVAL;
}

static int v4l_s_ctrl(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_control *p = arg;
	struct v4l2_fh *vfh =
		test_bit(V4L2_FL_USES_V4L2_FH, &vfd->flags) ? fh : NULL;
	struct v4l2_ext_controls ctrls;
	struct v4l2_ext_control ctrl;
	int ret;

	if (vfh && vfh->ctrl_handler)
		return v4l2_s_ctrl(vfh, vfh->ctrl_handler, p);
	if (vfd->ctrl_handler)
		return v4l2_s_ctrl(NULL, vfd->ctrl_handler, p);
	if (ops->vidioc_s_ctrl)
		return ops->vidioc_s_ctrl(file, fh, p);
	if (ops->vidioc_s_ext_ctrls == NULL)
		return -ENOTTY;

	ctrls.which = V4L2_CTRL_ID2WHICH(p->id);
	ctrls.count = 1;
	ctrls.controls = &ctrl;
	ctrl.id = p->id;
	ctrl.value = p->value;
	if (!check_ext_ctrls(&ctrls, VIDIOC_S_CTRL))
		return -EINVAL;
	ret = ops->vidioc_s_ext_ctrls(file, fh, &ctrls);
	p->value = ctrl.value;
	return ret;
}

static int v4l_g_ext_ctrls(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_ext_controls *p = arg;
	struct v4l2_fh *vfh =
		test_bit(V4L2_FL_USES_V4L2_FH, &vfd->flags) ? fh : NULL;

	p->error_idx = p->count;
	if (vfh && vfh->ctrl_handler)
		return v4l2_g_ext_ctrls(vfh->ctrl_handler,
					vfd, vfd->v4l2_dev->mdev, p);
	if (vfd->ctrl_handler)
		return v4l2_g_ext_ctrls(vfd->ctrl_handler,
					vfd, vfd->v4l2_dev->mdev, p);
	if (ops->vidioc_g_ext_ctrls == NULL)
		return -ENOTTY;
	return check_ext_ctrls(p, VIDIOC_G_EXT_CTRLS) ?
				ops->vidioc_g_ext_ctrls(file, fh, p) : -EINVAL;
}

static int v4l_s_ext_ctrls(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_ext_controls *p = arg;
	struct v4l2_fh *vfh =
		test_bit(V4L2_FL_USES_V4L2_FH, &vfd->flags) ? fh : NULL;

	p->error_idx = p->count;
	if (vfh && vfh->ctrl_handler)
		return v4l2_s_ext_ctrls(vfh, vfh->ctrl_handler,
					vfd, vfd->v4l2_dev->mdev, p);
	if (vfd->ctrl_handler)
		return v4l2_s_ext_ctrls(NULL, vfd->ctrl_handler,
					vfd, vfd->v4l2_dev->mdev, p);
	if (ops->vidioc_s_ext_ctrls == NULL)
		return -ENOTTY;
	return check_ext_ctrls(p, VIDIOC_S_EXT_CTRLS) ?
				ops->vidioc_s_ext_ctrls(file, fh, p) : -EINVAL;
}

static int v4l_try_ext_ctrls(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_ext_controls *p = arg;
	struct v4l2_fh *vfh =
		test_bit(V4L2_FL_USES_V4L2_FH, &vfd->flags) ? fh : NULL;

	p->error_idx = p->count;
	if (vfh && vfh->ctrl_handler)
		return v4l2_try_ext_ctrls(vfh->ctrl_handler,
					  vfd, vfd->v4l2_dev->mdev, p);
	if (vfd->ctrl_handler)
		return v4l2_try_ext_ctrls(vfd->ctrl_handler,
					  vfd, vfd->v4l2_dev->mdev, p);
	if (ops->vidioc_try_ext_ctrls == NULL)
		return -ENOTTY;
	return check_ext_ctrls(p, VIDIOC_TRY_EXT_CTRLS) ?
			ops->vidioc_try_ext_ctrls(file, fh, p) : -EINVAL;
}

/*
 * The selection API specified originally that the _MPLANE buffer types
 * shouldn't be used. The reasons for this are lost in the mists of time
 * (or just really crappy memories). Regardless, this is really annoying
 * for userspace. So to keep things simple we map _MPLANE buffer types
 * to their 'regular' counterparts before calling the driver. And we
 * restore it afterwards. This way applications can use either buffer
 * type and drivers don't need to check for both.
 */
static int v4l_g_selection(const struct v4l2_ioctl_ops *ops,
			   struct file *file, void *fh, void *arg)
{
	struct v4l2_selection *p = arg;
	u32 old_type = p->type;
	int ret;

	if (p->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
		p->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	else if (p->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
		p->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	ret = ops->vidioc_g_selection(file, fh, p);
	p->type = old_type;
	return ret;
}

static int v4l_s_selection(const struct v4l2_ioctl_ops *ops,
			   struct file *file, void *fh, void *arg)
{
	struct v4l2_selection *p = arg;
	u32 old_type = p->type;
	int ret;

	if (p->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
		p->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	else if (p->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
		p->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	ret = ops->vidioc_s_selection(file, fh, p);
	p->type = old_type;
	return ret;
}

static int v4l_g_crop(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_crop *p = arg;
	struct v4l2_selection s = {
		.type = p->type,
	};
	int ret;

	/* simulate capture crop using selection api */

	/* crop means compose for output devices */
	if (V4L2_TYPE_IS_OUTPUT(p->type))
		s.target = V4L2_SEL_TGT_COMPOSE;
	else
		s.target = V4L2_SEL_TGT_CROP;

	if (test_bit(V4L2_FL_QUIRK_INVERTED_CROP, &vfd->flags))
		s.target = s.target == V4L2_SEL_TGT_COMPOSE ?
			V4L2_SEL_TGT_CROP : V4L2_SEL_TGT_COMPOSE;

	ret = v4l_g_selection(ops, file, fh, &s);

	/* copying results to old structure on success */
	if (!ret)
		p->c = s.r;
	return ret;
}

static int v4l_s_crop(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_crop *p = arg;
	struct v4l2_selection s = {
		.type = p->type,
		.r = p->c,
	};

	/* simulate capture crop using selection api */

	/* crop means compose for output devices */
	if (V4L2_TYPE_IS_OUTPUT(p->type))
		s.target = V4L2_SEL_TGT_COMPOSE;
	else
		s.target = V4L2_SEL_TGT_CROP;

	if (test_bit(V4L2_FL_QUIRK_INVERTED_CROP, &vfd->flags))
		s.target = s.target == V4L2_SEL_TGT_COMPOSE ?
			V4L2_SEL_TGT_CROP : V4L2_SEL_TGT_COMPOSE;

	return v4l_s_selection(ops, file, fh, &s);
}

static int v4l_cropcap(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_cropcap *p = arg;
	struct v4l2_selection s = { .type = p->type };
	int ret = 0;

	/* setting trivial pixelaspect */
	p->pixelaspect.numerator = 1;
	p->pixelaspect.denominator = 1;

	if (s.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
		s.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	else if (s.type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
		s.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

	/*
	 * The determine_valid_ioctls() call already should ensure
	 * that this can never happen, but just in case...
	 */
	if (WARN_ON(!ops->vidioc_g_selection))
		return -ENOTTY;

	if (ops->vidioc_g_pixelaspect)
		ret = ops->vidioc_g_pixelaspect(file, fh, s.type,
						&p->pixelaspect);

	/*
	 * Ignore ENOTTY or ENOIOCTLCMD error returns, just use the
	 * square pixel aspect ratio in that case.
	 */
	if (ret && ret != -ENOTTY && ret != -ENOIOCTLCMD)
		return ret;

	/* Use g_selection() to fill in the bounds and defrect rectangles */

	/* obtaining bounds */
	if (V4L2_TYPE_IS_OUTPUT(p->type))
		s.target = V4L2_SEL_TGT_COMPOSE_BOUNDS;
	else
		s.target = V4L2_SEL_TGT_CROP_BOUNDS;

	if (test_bit(V4L2_FL_QUIRK_INVERTED_CROP, &vfd->flags))
		s.target = s.target == V4L2_SEL_TGT_COMPOSE_BOUNDS ?
			V4L2_SEL_TGT_CROP_BOUNDS : V4L2_SEL_TGT_COMPOSE_BOUNDS;

	ret = v4l_g_selection(ops, file, fh, &s);
	if (ret)
		return ret;
	p->bounds = s.r;

	/* obtaining defrect */
	if (s.target == V4L2_SEL_TGT_COMPOSE_BOUNDS)
		s.target = V4L2_SEL_TGT_COMPOSE_DEFAULT;
	else
		s.target = V4L2_SEL_TGT_CROP_DEFAULT;

	ret = v4l_g_selection(ops, file, fh, &s);
	if (ret)
		return ret;
	p->defrect = s.r;

	return 0;
}

static int v4l_log_status(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	int ret;

	if (vfd->v4l2_dev)
		pr_info("%s: =================  START STATUS  =================\n",
			vfd->v4l2_dev->name);
	ret = ops->vidioc_log_status(file, fh);
	if (vfd->v4l2_dev)
		pr_info("%s: ==================  END STATUS  ==================\n",
			vfd->v4l2_dev->name);
	return ret;
}

static int v4l_dbg_g_register(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
#ifdef CONFIG_VIDEO_ADV_DEBUG
	struct v4l2_dbg_register *p = arg;
	struct video_device *vfd = video_devdata(file);
	struct v4l2_subdev *sd;
	int idx = 0;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	if (p->match.type == V4L2_CHIP_MATCH_SUBDEV) {
		if (vfd->v4l2_dev == NULL)
			return -EINVAL;
		v4l2_device_for_each_subdev(sd, vfd->v4l2_dev)
			if (p->match.addr == idx++)
				return v4l2_subdev_call(sd, core, g_register, p);
		return -EINVAL;
	}
	if (ops->vidioc_g_register && p->match.type == V4L2_CHIP_MATCH_BRIDGE &&
	    (ops->vidioc_g_chip_info || p->match.addr == 0))
		return ops->vidioc_g_register(file, fh, p);
	return -EINVAL;
#else
	return -ENOTTY;
#endif
}

static int v4l_dbg_s_register(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
#ifdef CONFIG_VIDEO_ADV_DEBUG
	const struct v4l2_dbg_register *p = arg;
	struct video_device *vfd = video_devdata(file);
	struct v4l2_subdev *sd;
	int idx = 0;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	if (p->match.type == V4L2_CHIP_MATCH_SUBDEV) {
		if (vfd->v4l2_dev == NULL)
			return -EINVAL;
		v4l2_device_for_each_subdev(sd, vfd->v4l2_dev)
			if (p->match.addr == idx++)
				return v4l2_subdev_call(sd, core, s_register, p);
		return -EINVAL;
	}
	if (ops->vidioc_s_register && p->match.type == V4L2_CHIP_MATCH_BRIDGE &&
	    (ops->vidioc_g_chip_info || p->match.addr == 0))
		return ops->vidioc_s_register(file, fh, p);
	return -EINVAL;
#else
	return -ENOTTY;
#endif
}

static int v4l_dbg_g_chip_info(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
#ifdef CONFIG_VIDEO_ADV_DEBUG
	struct video_device *vfd = video_devdata(file);
	struct v4l2_dbg_chip_info *p = arg;
	struct v4l2_subdev *sd;
	int idx = 0;

	switch (p->match.type) {
	case V4L2_CHIP_MATCH_BRIDGE:
		if (ops->vidioc_s_register)
			p->flags |= V4L2_CHIP_FL_WRITABLE;
		if (ops->vidioc_g_register)
			p->flags |= V4L2_CHIP_FL_READABLE;
		strscpy(p->name, vfd->v4l2_dev->name, sizeof(p->name));
		if (ops->vidioc_g_chip_info)
			return ops->vidioc_g_chip_info(file, fh, arg);
		if (p->match.addr)
			return -EINVAL;
		return 0;

	case V4L2_CHIP_MATCH_SUBDEV:
		if (vfd->v4l2_dev == NULL)
			break;
		v4l2_device_for_each_subdev(sd, vfd->v4l2_dev) {
			if (p->match.addr != idx++)
				continue;
			if (sd->ops->core && sd->ops->core->s_register)
				p->flags |= V4L2_CHIP_FL_WRITABLE;
			if (sd->ops->core && sd->ops->core->g_register)
				p->flags |= V4L2_CHIP_FL_READABLE;
			strscpy(p->name, sd->name, sizeof(p->name));
			return 0;
		}
		break;
	}
	return -EINVAL;
#else
	return -ENOTTY;
#endif
}

static int v4l_dqevent(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	return v4l2_event_dequeue(fh, arg, file->f_flags & O_NONBLOCK);
}

static int v4l_subscribe_event(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	return ops->vidioc_subscribe_event(fh, arg);
}

static int v4l_unsubscribe_event(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	return ops->vidioc_unsubscribe_event(fh, arg);
}

static int v4l_g_sliced_vbi_cap(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct v4l2_sliced_vbi_cap *p = arg;
	int ret = check_fmt(file, p->type);

	if (ret)
		return ret;

	/* Clear up to type, everything after type is zeroed already */
	memset(p, 0, offsetof(struct v4l2_sliced_vbi_cap, type));

	return ops->vidioc_g_sliced_vbi_cap(file, fh, p);
}

static int v4l_enum_freq_bands(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_frequency_band *p = arg;
	enum v4l2_tuner_type type;
	int err;

	if (vfd->vfl_type == VFL_TYPE_SDR) {
		if (p->type != V4L2_TUNER_SDR && p->type != V4L2_TUNER_RF)
			return -EINVAL;
		type = p->type;
	} else {
		type = (vfd->vfl_type == VFL_TYPE_RADIO) ?
				V4L2_TUNER_RADIO : V4L2_TUNER_ANALOG_TV;
		if (type != p->type)
			return -EINVAL;
	}
	if (ops->vidioc_enum_freq_bands) {
		err = ops->vidioc_enum_freq_bands(file, fh, p);
		if (err != -ENOTTY)
			return err;
	}
	if (is_valid_ioctl(vfd, VIDIOC_G_TUNER)) {
		struct v4l2_tuner t = {
			.index = p->tuner,
			.type = type,
		};

		if (p->index)
			return -EINVAL;
		err = ops->vidioc_g_tuner(file, fh, &t);
		if (err)
			return err;
		p->capability = t.capability | V4L2_TUNER_CAP_FREQ_BANDS;
		p->rangelow = t.rangelow;
		p->rangehigh = t.rangehigh;
		p->modulation = (type == V4L2_TUNER_RADIO) ?
			V4L2_BAND_MODULATION_FM : V4L2_BAND_MODULATION_VSB;
		return 0;
	}
	if (is_valid_ioctl(vfd, VIDIOC_G_MODULATOR)) {
		struct v4l2_modulator m = {
			.index = p->tuner,
		};

		if (type != V4L2_TUNER_RADIO)
			return -EINVAL;
		if (p->index)
			return -EINVAL;
		err = ops->vidioc_g_modulator(file, fh, &m);
		if (err)
			return err;
		p->capability = m.capability | V4L2_TUNER_CAP_FREQ_BANDS;
		p->rangelow = m.rangelow;
		p->rangehigh = m.rangehigh;
		p->modulation = (type == V4L2_TUNER_RADIO) ?
			V4L2_BAND_MODULATION_FM : V4L2_BAND_MODULATION_VSB;
		return 0;
	}
	return -ENOTTY;
}

static int v4l_enuminput(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_input *p = arg;

	/*
	 * We set the flags for CAP_DV_TIMINGS &
	 * CAP_STD here based on ioctl handler provided by the
	 * driver. If the driver doesn't support these
	 * for a specific input, it must override these flags.
	 */
	if (is_valid_ioctl(vfd, VIDIOC_S_STD))
		p->capabilities |= V4L2_IN_CAP_STD;

	if (vfd->device_caps & V4L2_CAP_IO_MC) {
		if (p->index)
			return -EINVAL;
		strscpy(p->name, vfd->name, sizeof(p->name));
		p->type = V4L2_INPUT_TYPE_CAMERA;
		return 0;
	}

	return ops->vidioc_enum_input(file, fh, p);
}

static int v4l_g_input(const struct v4l2_ioctl_ops *ops,
		       struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);

	if (vfd->device_caps & V4L2_CAP_IO_MC) {
		*(int *)arg = 0;
		return 0;
	}

	return ops->vidioc_g_input(file, fh, arg);
}

static int v4l_s_input(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	int ret;

	ret = v4l_enable_media_source(vfd);
	if (ret)
		return ret;

	if (vfd->device_caps & V4L2_CAP_IO_MC)
		return  *(int *)arg ? -EINVAL : 0;

	return ops->vidioc_s_input(file, fh, *(unsigned int *)arg);
}

static int v4l_g_output(const struct v4l2_ioctl_ops *ops,
			struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);

	if (vfd->device_caps & V4L2_CAP_IO_MC) {
		*(int *)arg = 0;
		return 0;
	}

	return ops->vidioc_g_output(file, fh, arg);
}

static int v4l_s_output(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);

	if (vfd->device_caps & V4L2_CAP_IO_MC)
		return  *(int *)arg ? -EINVAL : 0;

	return ops->vidioc_s_output(file, fh, *(unsigned int *)arg);
}

static int v4l_enumoutput(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd = video_devdata(file);
	struct v4l2_output *p = arg;

	/*
	 * We set the flags for CAP_DV_TIMINGS &
	 * CAP_STD here based on ioctl handler provided by the
	 * driver. If the driver doesn't support these
	 * for a specific output, it must override these flags.
	 */
	if (is_valid_ioctl(vfd, VIDIOC_S_STD))
		p->capabilities |= V4L2_OUT_CAP_STD;

	if (vfd->device_caps & V4L2_CAP_IO_MC) {
		if (p->index)
			return -EINVAL;
		strscpy(p->name, vfd->name, sizeof(p->name));
		p->type = V4L2_OUTPUT_TYPE_ANALOG;
		return 0;
	}

	return ops->vidioc_enum_output(file, fh, p);
}

static int v4l_g_priority(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd;
	u32 *p = arg;

	vfd = video_devdata(file);
	*p = v4l2_prio_max(vfd->prio);
	return 0;
}

static int v4l_s_priority(const struct v4l2_ioctl_ops *ops,
				struct file *file, void *fh, void *arg)
{
	struct video_device *vfd;
	struct v4l2_fh *vfh;
	u32 *p = arg;

	vfd = video_devdata(file);
	if (!test_bit(V4L2_FL_USES_V4L2_FH, &vfd->flags))
		return -ENOTTY;
	vfh = file->private_data;
	return v4l2_prio_change(vfd->prio, &vfh->prio, *p);
}




DEFINE_V4L_STUB_FUNC(g_fbuf)
DEFINE_V4L_STUB_FUNC(s_fbuf)
DEFINE_V4L_STUB_FUNC(expbuf)
DEFINE_V4L_STUB_FUNC(g_std)
DEFINE_V4L_STUB_FUNC(g_audio)
DEFINE_V4L_STUB_FUNC(s_audio)
DEFINE_V4L_STUB_FUNC(g_edid)
DEFINE_V4L_STUB_FUNC(s_edid)
DEFINE_V4L_STUB_FUNC(g_audout)
DEFINE_V4L_STUB_FUNC(s_audout)
DEFINE_V4L_STUB_FUNC(g_jpegcomp)
DEFINE_V4L_STUB_FUNC(s_jpegcomp)
DEFINE_V4L_STUB_FUNC(enumaudio)
DEFINE_V4L_STUB_FUNC(enumaudout)
DEFINE_V4L_STUB_FUNC(enum_framesizes)
DEFINE_V4L_STUB_FUNC(enum_frameintervals)
DEFINE_V4L_STUB_FUNC(g_enc_index)
DEFINE_V4L_STUB_FUNC(encoder_cmd)
DEFINE_V4L_STUB_FUNC(try_encoder_cmd)
DEFINE_V4L_STUB_FUNC(decoder_cmd)
DEFINE_V4L_STUB_FUNC(try_decoder_cmd)
DEFINE_V4L_STUB_FUNC(s_dv_timings)
DEFINE_V4L_STUB_FUNC(g_dv_timings)
DEFINE_V4L_STUB_FUNC(enum_dv_timings)
DEFINE_V4L_STUB_FUNC(query_dv_timings)
DEFINE_V4L_STUB_FUNC(dv_timings_cap)


static const struct v4l2_ioctl_info v4l2_ioctls[] = {
	IOCTL_INFO(VIDIOC_QUERYCAP, v4l_querycap, v4l_print_querycap, 0),
	IOCTL_INFO(VIDIOC_ENUM_FMT, v4l_enum_fmt, v4l_print_fmtdesc, 0),
	IOCTL_INFO(VIDIOC_G_FMT, v4l_g_fmt, v4l_print_format, 0),
	IOCTL_INFO(VIDIOC_S_FMT, v4l_s_fmt, v4l_print_format, INFO_FL_PRIO),
	IOCTL_INFO(VIDIOC_REQBUFS, v4l_reqbufs, v4l_print_requestbuffers, INFO_FL_PRIO | INFO_FL_QUEUE),
	IOCTL_INFO(VIDIOC_QUERYBUF, v4l_querybuf, v4l_print_buffer, INFO_FL_QUEUE | INFO_FL_CLEAR(v4l2_buffer, length)),
	IOCTL_INFO(VIDIOC_G_FBUF, v4l_stub_g_fbuf, v4l_print_framebuffer, 0),
	IOCTL_INFO(VIDIOC_S_FBUF, v4l_stub_s_fbuf, v4l_print_framebuffer, INFO_FL_PRIO),
	IOCTL_INFO(VIDIOC_OVERLAY, v4l_overlay, v4l_print_u32, INFO_FL_PRIO),
	IOCTL_INFO(VIDIOC_QBUF, v4l_qbuf, v4l_print_buffer, INFO_FL_QUEUE),
	IOCTL_INFO(VIDIOC_EXPBUF, v4l_stub_expbuf, v4l_print_exportbuffer, INFO_FL_QUEUE | INFO_FL_CLEAR(v4l2_exportbuffer, flags)),
	IOCTL_INFO(VIDIOC_DQBUF, v4l_dqbuf, v4l_print_buffer, INFO_FL_QUEUE),
	IOCTL_INFO(VIDIOC_STREAMON, v4l_streamon, v4l_print_buftype, INFO_FL_PRIO | INFO_FL_QUEUE),
	IOCTL_INFO(VIDIOC_STREAMOFF, v4l_streamoff, v4l_print_buftype, INFO_FL_PRIO | INFO_FL_QUEUE),
	IOCTL_INFO(VIDIOC_G_PARM, v4l_g_parm, v4l_print_streamparm, INFO_FL_CLEAR(v4l2_streamparm, type)),
	IOCTL_INFO(VIDIOC_S_PARM, v4l_s_parm, v4l_print_streamparm, INFO_FL_PRIO),
	IOCTL_INFO(VIDIOC_G_STD, v4l_stub_g_std, v4l_print_std, 0),
	IOCTL_INFO(VIDIOC_S_STD, v4l_s_std, v4l_print_std, INFO_FL_PRIO),
	IOCTL_INFO(VIDIOC_ENUMSTD, v4l_enumstd, v4l_print_standard, INFO_FL_CLEAR(v4l2_standard, index)),
	IOCTL_INFO(VIDIOC_ENUMINPUT, v4l_enuminput, v4l_print_enuminput, INFO_FL_CLEAR(v4l2_input, index)),
	IOCTL_INFO(VIDIOC_G_CTRL, v4l_g_ctrl, v4l_print_control, INFO_FL_CTRL | INFO_FL_CLEAR(v4l2_control, id)),
	IOCTL_INFO(VIDIOC_S_CTRL, v4l_s_ctrl, v4l_print_control, INFO_FL_PRIO | INFO_FL_CTRL),
	IOCTL_INFO(VIDIOC_G_TUNER, v4l_g_tuner, v4l_print_tuner, INFO_FL_CLEAR(v4l2_tuner, index)),
	IOCTL_INFO(VIDIOC_S_TUNER, v4l_s_tuner, v4l_print_tuner, INFO_FL_PRIO),
	IOCTL_INFO(VIDIOC_G_AUDIO, v4l_stub_g_audio, v4l_print_audio, 0),
	IOCTL_INFO(VIDIOC_S_AUDIO, v4l_stub_s_audio, v4l_print_audio, INFO_FL_PRIO),
	IOCTL_INFO(VIDIOC_QUERYCTRL, v4l_queryctrl, v4l_print_queryctrl, INFO_FL_CTRL | INFO_FL_CLEAR(v4l2_queryctrl, id)),
	IOCTL_INFO(VIDIOC_QUERYMENU, v4l_querymenu, v4l_print_querymenu, INFO_FL_CTRL | INFO_FL_CLEAR(v4l2_querymenu, index)),
	IOCTL_INFO(VIDIOC_G_INPUT, v4l_g_input, v4l_print_u32, 0),
	IOCTL_INFO(VIDIOC_S_INPUT, v4l_s_input, v4l_print_u32, INFO_FL_PRIO),
	IOCTL_INFO(VIDIOC_G_EDID, v4l_stub_g_edid, v4l_print_edid, INFO_FL_ALWAYS_COPY),
	IOCTL_INFO(VIDIOC_S_EDID, v4l_stub_s_edid, v4l_print_edid, INFO_FL_PRIO | INFO_FL_ALWAYS_COPY),
	IOCTL_INFO(VIDIOC_G_OUTPUT, v4l_g_output, v4l_print_u32, 0),
	IOCTL_INFO(VIDIOC_S_OUTPUT, v4l_s_output, v4l_print_u32, INFO_FL_PRIO),
	IOCTL_INFO(VIDIOC_ENUMOUTPUT, v4l_enumoutput, v4l_print_enumoutput, INFO_FL_CLEAR(v4l2_output, index)),
	IOCTL_INFO(VIDIOC_G_AUDOUT, v4l_stub_g_audout, v4l_print_audioout, 0),
	IOCTL_INFO(VIDIOC_S_AUDOUT, v4l_stub_s_audout, v4l_print_audioout, INFO_FL_PRIO),
	IOCTL_INFO(VIDIOC_G_MODULATOR, v4l_g_modulator, v4l_print_modulator, INFO_FL_CLEAR(v4l2_modulator, index)),
	IOCTL_INFO(VIDIOC_S_MODULATOR, v4l_s_modulator, v4l_print_modulator, INFO_FL_PRIO),
	IOCTL_INFO(VIDIOC_G_FREQUENCY, v4l_g_frequency, v4l_print_frequency, INFO_FL_CLEAR(v4l2_frequency, tuner)),
	IOCTL_INFO(VIDIOC_S_FREQUENCY, v4l_s_frequency, v4l_print_frequency, INFO_FL_PRIO),
	IOCTL_INFO(VIDIOC_CROPCAP, v4l_cropcap, v4l_print_cropcap, INFO_FL_CLEAR(v4l2_cropcap, type)),
	IOCTL_INFO(VIDIOC_G_CROP, v4l_g_crop, v4l_print_crop, INFO_FL_CLEAR(v4l2_crop, type)),
	IOCTL_INFO(VIDIOC_S_CROP, v4l_s_crop, v4l_print_crop, INFO_FL_PRIO),
	IOCTL_INFO(VIDIOC_G_SELECTION, v4l_g_selection, v4l_print_selection, INFO_FL_CLEAR(v4l2_selection, r)),
	IOCTL_INFO(VIDIOC_S_SELECTION, v4l_s_selection, v4l_print_selection, INFO_FL_PRIO | INFO_FL_CLEAR(v4l2_selection, r)),
	IOCTL_INFO(VIDIOC_G_JPEGCOMP, v4l_stub_g_jpegcomp, v4l_print_jpegcompression, 0),
	IOCTL_INFO(VIDIOC_S_JPEGCOMP, v4l_stub_s_jpegcomp, v4l_print_jpegcompression, INFO_FL_PRIO),
	IOCTL_INFO(VIDIOC_QUERYSTD, v4l_querystd, v4l_print_std, 0),
	IOCTL_INFO(VIDIOC_TRY_FMT, v4l_try_fmt, v4l_print_format, 0),
	IOCTL_INFO(VIDIOC_ENUMAUDIO, v4l_stub_enumaudio, v4l_print_audio, INFO_FL_CLEAR(v4l2_audio, index)),
	IOCTL_INFO(VIDIOC_ENUMAUDOUT, v4l_stub_enumaudout, v4l_print_audioout, INFO_FL_CLEAR(v4l2_audioout, index)),
	IOCTL_INFO(VIDIOC_G_PRIORITY, v4l_g_priority, v4l_print_u32, 0),
	IOCTL_INFO(VIDIOC_S_PRIORITY, v4l_s_priority, v4l_print_u32, INFO_FL_PRIO),
	IOCTL_INFO(VIDIOC_G_SLICED_VBI_CAP, v4l_g_sliced_vbi_cap, v4l_print_sliced_vbi_cap, INFO_FL_CLEAR(v4l2_sliced_vbi_cap, type)),
	IOCTL_INFO(VIDIOC_LOG_STATUS, v4l_log_status, v4l_print_newline, 0),
	IOCTL_INFO(VIDIOC_G_EXT_CTRLS, v4l_g_ext_ctrls, v4l_print_ext_controls, INFO_FL_CTRL),
	IOCTL_INFO(VIDIOC_S_EXT_CTRLS, v4l_s_ext_ctrls, v4l_print_ext_controls, INFO_FL_PRIO | INFO_FL_CTRL),
	IOCTL_INFO(VIDIOC_TRY_EXT_CTRLS, v4l_try_ext_ctrls, v4l_print_ext_controls, INFO_FL_CTRL),
	IOCTL_INFO(VIDIOC_ENUM_FRAMESIZES, v4l_stub_enum_framesizes, v4l_print_frmsizeenum, INFO_FL_CLEAR(v4l2_frmsizeenum, pixel_format)),
	IOCTL_INFO(VIDIOC_ENUM_FRAMEINTERVALS, v4l_stub_enum_frameintervals, v4l_print_frmivalenum, INFO_FL_CLEAR(v4l2_frmivalenum, height)),
	IOCTL_INFO(VIDIOC_G_ENC_INDEX, v4l_stub_g_enc_index, v4l_print_enc_idx, 0),
	IOCTL_INFO(VIDIOC_ENCODER_CMD, v4l_stub_encoder_cmd, v4l_print_encoder_cmd, INFO_FL_PRIO | INFO_FL_CLEAR(v4l2_encoder_cmd, flags)),
	IOCTL_INFO(VIDIOC_TRY_ENCODER_CMD, v4l_stub_try_encoder_cmd, v4l_print_encoder_cmd, INFO_FL_CLEAR(v4l2_encoder_cmd, flags)),
	IOCTL_INFO(VIDIOC_DECODER_CMD, v4l_stub_decoder_cmd, v4l_print_decoder_cmd, INFO_FL_PRIO),
	IOCTL_INFO(VIDIOC_TRY_DECODER_CMD, v4l_stub_try_decoder_cmd, v4l_print_decoder_cmd, 0),
	IOCTL_INFO(VIDIOC_DBG_S_REGISTER, v4l_dbg_s_register, v4l_print_dbg_register, 0),
	IOCTL_INFO(VIDIOC_DBG_G_REGISTER, v4l_dbg_g_register, v4l_print_dbg_register, 0),
	IOCTL_INFO(VIDIOC_S_HW_FREQ_SEEK, v4l_s_hw_freq_seek, v4l_print_hw_freq_seek, INFO_FL_PRIO),
	IOCTL_INFO(VIDIOC_S_DV_TIMINGS, v4l_stub_s_dv_timings, v4l_print_dv_timings, INFO_FL_PRIO | INFO_FL_CLEAR(v4l2_dv_timings, bt.flags)),
	IOCTL_INFO(VIDIOC_G_DV_TIMINGS, v4l_stub_g_dv_timings, v4l_print_dv_timings, 0),
	IOCTL_INFO(VIDIOC_DQEVENT, v4l_dqevent, v4l_print_event, 0),
	IOCTL_INFO(VIDIOC_SUBSCRIBE_EVENT, v4l_subscribe_event, v4l_print_event_subscription, 0),
	IOCTL_INFO(VIDIOC_UNSUBSCRIBE_EVENT, v4l_unsubscribe_event, v4l_print_event_subscription, 0),
	IOCTL_INFO(VIDIOC_CREATE_BUFS, v4l_create_bufs, v4l_print_create_buffers, INFO_FL_PRIO | INFO_FL_QUEUE),
	IOCTL_INFO(VIDIOC_PREPARE_BUF, v4l_prepare_buf, v4l_print_buffer, INFO_FL_QUEUE),
	IOCTL_INFO(VIDIOC_ENUM_DV_TIMINGS, v4l_stub_enum_dv_timings, v4l_print_enum_dv_timings, INFO_FL_CLEAR(v4l2_enum_dv_timings, pad)),
	IOCTL_INFO(VIDIOC_QUERY_DV_TIMINGS, v4l_stub_query_dv_timings, v4l_print_dv_timings, INFO_FL_ALWAYS_COPY),
	IOCTL_INFO(VIDIOC_DV_TIMINGS_CAP, v4l_stub_dv_timings_cap, v4l_print_dv_timings_cap, INFO_FL_CLEAR(v4l2_dv_timings_cap, pad)),
	IOCTL_INFO(VIDIOC_ENUM_FREQ_BANDS, v4l_enum_freq_bands, v4l_print_freq_band, 0),
	IOCTL_INFO(VIDIOC_DBG_G_CHIP_INFO, v4l_dbg_g_chip_info, v4l_print_dbg_chip_info, INFO_FL_CLEAR(v4l2_dbg_chip_info, match)),

	IOCTL_INFO(VIDIOC_QUERY_EXT_CTRL, v4l_query_ext_ctrl, v4l_print_query_ext_ctrl, INFO_FL_CTRL | INFO_FL_CLEAR(v4l2_query_ext_ctrl, id)),
};
#define V4L2_IOCTLS ARRAY_SIZE(v4l2_ioctls)

static struct mutex *v4l2_ioctl_get_lock(struct video_device *vdev,
					 struct v4l2_fh *vfh, unsigned int cmd,
					 void *arg)
{
	
	if (_IOC_NR(cmd) >= V4L2_IOCTLS)
		return vdev->lock;
	if (vfh && vfh->m2m_ctx &&
	    (v4l2_ioctls[_IOC_NR(cmd)].flags & INFO_FL_QUEUE)) {
		if (vfh->m2m_ctx->q_lock)
			return vfh->m2m_ctx->q_lock;
	}

	if (vdev->queue && vdev->queue->lock &&
			(v4l2_ioctls[_IOC_NR(cmd)].flags & INFO_FL_QUEUE))
		return vdev->queue->lock;



	return vdev->lock;
}


static bool v4l2_is_known_ioctl(unsigned int cmd)
{
	if (_IOC_NR(cmd) >= V4L2_IOCTLS)
		return false;
	return v4l2_ioctls[_IOC_NR(cmd)].ioctl == cmd;
}


static long __video_do_ioctl(struct file *file,
		unsigned int cmd, void *arg)
{
	printk(KERN_ALERT "__video_do_ioctl : enter!\r\n");
	struct video_device *vfd = video_devdata(file);
	printk(KERN_ALERT "vfd == 0x%lx!\r\n" , (unsigned long)vfd);
	struct mutex *req_queue_lock = NULL;
	struct mutex *lock; /* ioctl serialization mutex */
	const struct v4l2_ioctl_ops *ops = vfd->ioctl_ops;
	bool write_only = false;
	struct v4l2_ioctl_info default_info;
	const struct v4l2_ioctl_info *info;
	void *fh = file->private_data;
	struct v4l2_fh *vfh = NULL;
	int dev_debug = vfd->dev_debug;
	long ret = -ENOTTY;


	if (ops == NULL) {
		pr_warn("%s: has no ioctl_ops.\n",
				video_device_node_name(vfd));
		printk(KERN_ALERT "ops == NULL!\r\n" , ret);
		return ret;
	}

	if (test_bit(V4L2_FL_USES_V4L2_FH, &vfd->flags))
	{
		printk(KERN_ALERT "test_bit : enter!\r\n");
		vfh = file->private_data;
	}
	/*
	 * We need to serialize streamon/off with queueing new requests.
	 * These ioctls may trigger the cancellation of a streaming
	 * operation, and that should not be mixed with queueing a new
	 * request at the same time.
	 */
	if (v4l2_device_supports_requests(vfd->v4l2_dev) &&
	    (cmd == VIDIOC_STREAMON || cmd == VIDIOC_STREAMOFF)) {
		req_queue_lock = &vfd->v4l2_dev->mdev->req_queue_mutex;
		printk(KERN_ALERT "v4l2_device_supports_requests : enter!\r\n");
		if (mutex_lock_interruptible(req_queue_lock))
			return -ERESTARTSYS;
	}

	lock = v4l2_ioctl_get_lock(vfd, vfh, cmd, arg);

	if (lock && mutex_lock_interruptible(lock)) {
		printk(KERN_ALERT "mutex_lock_interruptible!\r\n");

		if (req_queue_lock)
			mutex_unlock(req_queue_lock);
		return -ERESTARTSYS;
	}

	if (!video_is_registered(vfd)) {
		printk(KERN_ALERT "!video_is_registered : enter!\r\n");
		ret = -ENODEV;
		goto unlock;
	}

	//ret = v4l2_is_known_ioctl(cmd);
	//printk(KERN_ALERT "ret = %d!\r\n" , ret);
	if (v4l2_is_known_ioctl(cmd)) {
		info = &v4l2_ioctls[_IOC_NR(cmd)];
		printk(KERN_ALERT "_IOC_NR(cmd) = %d , vfd->valid_ioctls = %d!\r\n" , _IOC_NR(cmd) , vfd->valid_ioctls);

		if (!test_bit(_IOC_NR(cmd), vfd->valid_ioctls) &&
		    !((info->flags & INFO_FL_CTRL) && vfh && vfh->ctrl_handler))
		{
			printk(KERN_ALERT "!test_bit\r\n");
			goto done;
		}

		if (vfh && (info->flags & INFO_FL_PRIO)) {
			ret = v4l2_prio_check(vfd->prio, vfh->prio);
			printk(KERN_ALERT "v4l2_prio_check : ret = %d!\r\n" , ret);
			if (ret)
				goto done;
		}
	} else {
		default_info.ioctl = cmd;
		default_info.flags = 0;
		default_info.debug = v4l_print_default;
		info = &default_info;
	}

	printk(KERN_ALERT "v4l2_is_known_ioctl finish!\r\n");

	write_only = _IOC_DIR(cmd) == _IOC_WRITE;
	if (info != &default_info) {
		printk(KERN_ALERT "func : enter!\r\n");
		ret = info->func(ops, file, fh, arg);
		printk(KERN_ALERT "func : ret = %d!\r\n" , ret);
	} else if (!ops->vidioc_default) {
		ret = -ENOTTY;
	} else {
		ret = ops->vidioc_default(file, fh,
			vfh ? v4l2_prio_check(vfd->prio, vfh->prio) >= 0 : 0,
			cmd, arg);
	}

done:
	if (dev_debug & (V4L2_DEV_DEBUG_IOCTL | V4L2_DEV_DEBUG_IOCTL_ARG)) {
		if (!(dev_debug & V4L2_DEV_DEBUG_STREAMING) &&
		    (cmd == VIDIOC_QBUF || cmd == VIDIOC_DQBUF))
			goto unlock;

		v4l_printk_ioctl(video_device_node_name(vfd), cmd);
		if (ret < 0)
			pr_cont(": error %ld", ret);
		if (!(dev_debug & V4L2_DEV_DEBUG_IOCTL_ARG))
			pr_cont("\n");
		else if (_IOC_DIR(cmd) == _IOC_NONE)
			info->debug(arg, write_only);
		else {
			pr_cont(": ");
			info->debug(arg, write_only);
		}
	}


unlock:
	if (lock)
		mutex_unlock(lock);
	if (req_queue_lock)
		mutex_unlock(req_queue_lock);


	printk(KERN_ALERT "ret = %d!\r\n" , ret);
	return ret;

}

unsigned int v4l2_compat_translate_cmd(unsigned int cmd)
{
	switch (cmd) {
	case VIDIOC_G_FMT32:
		return VIDIOC_G_FMT;
	case VIDIOC_S_FMT32:
		return VIDIOC_S_FMT;
	case VIDIOC_TRY_FMT32:
		return VIDIOC_TRY_FMT;
	case VIDIOC_G_FBUF32:
		return VIDIOC_G_FBUF;
	case VIDIOC_S_FBUF32:
		return VIDIOC_S_FBUF;
#ifdef CONFIG_COMPAT_32BIT_TIME
	case VIDIOC_QUERYBUF32_TIME32:
		return VIDIOC_QUERYBUF;
	case VIDIOC_QBUF32_TIME32:
		return VIDIOC_QBUF;
	case VIDIOC_DQBUF32_TIME32:
		return VIDIOC_DQBUF;
	case VIDIOC_PREPARE_BUF32_TIME32:
		return VIDIOC_PREPARE_BUF;
#endif
	case VIDIOC_QUERYBUF32:
		return VIDIOC_QUERYBUF;
	case VIDIOC_QBUF32:
		return VIDIOC_QBUF;
	case VIDIOC_DQBUF32:
		return VIDIOC_DQBUF;
	case VIDIOC_CREATE_BUFS32:
		return VIDIOC_CREATE_BUFS;
	case VIDIOC_G_EXT_CTRLS32:
		return VIDIOC_G_EXT_CTRLS;
	case VIDIOC_S_EXT_CTRLS32:
		return VIDIOC_S_EXT_CTRLS;
	case VIDIOC_TRY_EXT_CTRLS32:
		return VIDIOC_TRY_EXT_CTRLS;
	case VIDIOC_PREPARE_BUF32:
		return VIDIOC_PREPARE_BUF;
	case VIDIOC_ENUMSTD32:
		return VIDIOC_ENUMSTD;
	case VIDIOC_ENUMINPUT32:
		return VIDIOC_ENUMINPUT;
	case VIDIOC_G_EDID32:
		return VIDIOC_G_EDID;
	case VIDIOC_S_EDID32:
		return VIDIOC_S_EDID;
#ifdef CONFIG_X86_64
	case VIDIOC_DQEVENT32:
		return VIDIOC_DQEVENT;
#endif
#ifdef CONFIG_COMPAT_32BIT_TIME
	case VIDIOC_DQEVENT32_TIME32:
		return VIDIOC_DQEVENT;
#endif
	}
	return cmd;
}


static unsigned int video_translate_cmd(unsigned int cmd)
{

#if !defined(CONFIG_64BIT) && defined(CONFIG_COMPAT_32BIT_TIME)
	switch (cmd) {
	case VIDIOC_DQEVENT_TIME32:
		return VIDIOC_DQEVENT;
	case VIDIOC_QUERYBUF_TIME32:
		return VIDIOC_QUERYBUF;
	case VIDIOC_QBUF_TIME32:
		return VIDIOC_QBUF;
	case VIDIOC_DQBUF_TIME32:
		return VIDIOC_DQBUF;
	case VIDIOC_PREPARE_BUF_TIME32:
		return VIDIOC_PREPARE_BUF;
	}
#endif
	if (in_compat_syscall())
		return v4l2_compat_translate_cmd(cmd);


	return cmd;
}

static int get_v4l2_format32(struct v4l2_format *p64,
			     struct v4l2_format32 __user *p32)
{
	if (get_user(p64->type, &p32->type))
		return -EFAULT;

	switch (p64->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		return copy_from_user(&p64->fmt.pix, &p32->fmt.pix,
				      sizeof(p64->fmt.pix)) ? -EFAULT : 0;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		return copy_from_user(&p64->fmt.pix_mp, &p32->fmt.pix_mp,
				      sizeof(p64->fmt.pix_mp)) ? -EFAULT : 0;
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
		return get_v4l2_window32(&p64->fmt.win, &p32->fmt.win);
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
		return copy_from_user(&p64->fmt.vbi, &p32->fmt.vbi,
				      sizeof(p64->fmt.vbi)) ? -EFAULT : 0;
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		return copy_from_user(&p64->fmt.sliced, &p32->fmt.sliced,
				      sizeof(p64->fmt.sliced)) ? -EFAULT : 0;
	case V4L2_BUF_TYPE_SDR_CAPTURE:
	case V4L2_BUF_TYPE_SDR_OUTPUT:
		return copy_from_user(&p64->fmt.sdr, &p32->fmt.sdr,
				      sizeof(p64->fmt.sdr)) ? -EFAULT : 0;
	case V4L2_BUF_TYPE_META_CAPTURE:
	case V4L2_BUF_TYPE_META_OUTPUT:
		return copy_from_user(&p64->fmt.meta, &p32->fmt.meta,
				      sizeof(p64->fmt.meta)) ? -EFAULT : 0;
	default:
		return -EINVAL;
	}
}


static int get_v4l2_framebuffer32(struct v4l2_framebuffer *p64,
				  struct v4l2_framebuffer32 __user *p32)
{
	compat_caddr_t tmp;

	if (get_user(tmp, &p32->base) ||
	    get_user(p64->capability, &p32->capability) ||
	    get_user(p64->flags, &p32->flags) ||
	    copy_from_user(&p64->fmt, &p32->fmt, sizeof(p64->fmt)))
		return -EFAULT;
	p64->base = (void __force *)compat_ptr(tmp);

	return 0;
}


#ifdef CONFIG_COMPAT_32BIT_TIME
static int get_v4l2_buffer32_time32(struct v4l2_buffer *vb,
				    struct v4l2_buffer32_time32 __user *arg)
{
	struct v4l2_buffer32_time32 vb32;

	if (copy_from_user(&vb32, arg, sizeof(vb32)))
		return -EFAULT;

	*vb = (struct v4l2_buffer) {
		.index		= vb32.index,
		.type		= vb32.type,
		.bytesused	= vb32.bytesused,
		.flags		= vb32.flags,
		.field		= vb32.field,
		.timestamp.tv_sec	= vb32.timestamp.tv_sec,
		.timestamp.tv_usec	= vb32.timestamp.tv_usec,
		.timecode	= vb32.timecode,
		.sequence	= vb32.sequence,
		.memory		= vb32.memory,
		.m.offset	= vb32.m.offset,
		.length		= vb32.length,
		.request_fd	= vb32.request_fd,
	};
	switch (vb->memory) {
	case V4L2_MEMORY_MMAP:
	case V4L2_MEMORY_OVERLAY:
		vb->m.offset = vb32.m.offset;
		break;
	case V4L2_MEMORY_USERPTR:
		vb->m.userptr = (unsigned long)compat_ptr(vb32.m.userptr);
		break;
	case V4L2_MEMORY_DMABUF:
		vb->m.fd = vb32.m.fd;
		break;
	}

	if (V4L2_TYPE_IS_MULTIPLANAR(vb->type))
		vb->m.planes = (void __force *)
				compat_ptr(vb32.m.planes);

	return 0;
}
#endif

static int get_v4l2_buffer32(struct v4l2_buffer *vb,
			     struct v4l2_buffer32 __user *arg)
{
	struct v4l2_buffer32 vb32;

	if (copy_from_user(&vb32, arg, sizeof(vb32)))
		return -EFAULT;

	memset(vb, 0, sizeof(*vb));
	*vb = (struct v4l2_buffer) {
		.index		= vb32.index,
		.type		= vb32.type,
		.bytesused	= vb32.bytesused,
		.flags		= vb32.flags,
		.field		= vb32.field,
		.timestamp.tv_sec	= vb32.timestamp.tv_sec,
		.timestamp.tv_usec	= vb32.timestamp.tv_usec,
		.timecode	= vb32.timecode,
		.sequence	= vb32.sequence,
		.memory		= vb32.memory,
		.m.offset	= vb32.m.offset,
		.length		= vb32.length,
		.request_fd	= vb32.request_fd,
	};

	switch (vb->memory) {
	case V4L2_MEMORY_MMAP:
	case V4L2_MEMORY_OVERLAY:
		vb->m.offset = vb32.m.offset;
		break;
	case V4L2_MEMORY_USERPTR:
		vb->m.userptr = (unsigned long)compat_ptr(vb32.m.userptr);
		break;
	case V4L2_MEMORY_DMABUF:
		vb->m.fd = vb32.m.fd;
		break;
	}

	if (V4L2_TYPE_IS_MULTIPLANAR(vb->type))
		vb->m.planes = (void __force *)
				compat_ptr(vb32.m.planes);

	return 0;
}


static int get_v4l2_ext_controls32(struct v4l2_ext_controls *p64,
				   struct v4l2_ext_controls32 __user *p32)
{
	struct v4l2_ext_controls32 ec32;

	if (copy_from_user(&ec32, p32, sizeof(ec32)))
		return -EFAULT;

	*p64 = (struct v4l2_ext_controls) {
		.which		= ec32.which,
		.count		= ec32.count,
		.error_idx	= ec32.error_idx,
		.request_fd	= ec32.request_fd,
		.reserved[0]	= ec32.reserved[0],
		.controls	= (void __force *)compat_ptr(ec32.controls),
	};

	return 0;
}

static int get_v4l2_create32(struct v4l2_create_buffers *p64,
			     struct v4l2_create_buffers32 __user *p32)
{
	if (copy_from_user(p64, p32,
			   offsetof(struct v4l2_create_buffers32, format)))
		return -EFAULT;
	return get_v4l2_format32(&p64->format, &p32->format);
}




int v4l2_compat_get_user(void __user *arg, void *parg, unsigned int cmd)
{


	switch (cmd) {
	case VIDIOC_G_FMT32:
	case VIDIOC_S_FMT32:
	case VIDIOC_TRY_FMT32:
		return get_v4l2_format32(parg, arg);

	case VIDIOC_S_FBUF32:
		return get_v4l2_framebuffer32(parg, arg);
#ifdef CONFIG_COMPAT_32BIT_TIME
	case VIDIOC_QUERYBUF32_TIME32:
	case VIDIOC_QBUF32_TIME32:
	case VIDIOC_DQBUF32_TIME32:
	case VIDIOC_PREPARE_BUF32_TIME32:
		return get_v4l2_buffer32_time32(parg, arg);
#endif
	case VIDIOC_QUERYBUF32:
	case VIDIOC_QBUF32:
	case VIDIOC_DQBUF32:
	case VIDIOC_PREPARE_BUF32:
		return get_v4l2_buffer32(parg, arg);

	case VIDIOC_G_EXT_CTRLS32:
	case VIDIOC_S_EXT_CTRLS32:
	case VIDIOC_TRY_EXT_CTRLS32:
		return get_v4l2_ext_controls32(parg, arg);

	case VIDIOC_CREATE_BUFS32:
		return get_v4l2_create32(parg, arg);

	case VIDIOC_ENUMSTD32:
		return get_v4l2_standard32(parg, arg);

	case VIDIOC_ENUMINPUT32:
		return get_v4l2_input32(parg, arg);

	case VIDIOC_G_EDID32:
	case VIDIOC_S_EDID32:
		return get_v4l2_edid32(parg, arg);
	}


	return 0;
}

static int video_get_user(void __user *arg, void *parg,
			  unsigned int real_cmd, unsigned int cmd,
			  bool *always_copy)
{

	
	unsigned int n = _IOC_SIZE(real_cmd);
	int err = 0;

	if (!(_IOC_DIR(cmd) & _IOC_WRITE)) {
		/* read-only ioctl */
		memset(parg, 0, n);
		return 0;
	}



	/*
	 * In some cases, only a few fields are used as input,
	 * i.e. when the app sets "index" and then the driver
	 * fills in the rest of the structure for the thing
	 * with that index.  We only need to copy up the first
	 * non-input field.
	 */
	if (v4l2_is_known_ioctl(real_cmd)) {
		u32 flags = v4l2_ioctls[_IOC_NR(real_cmd)].flags;

		if (flags & INFO_FL_CLEAR_MASK)
			n = (flags & INFO_FL_CLEAR_MASK) >> 16;
		*always_copy = flags & INFO_FL_ALWAYS_COPY;
	}



	if (cmd == real_cmd) {
		if (copy_from_user(parg, (void __user *)arg, n))
			err = -EFAULT;
	} else if (in_compat_syscall()) {
		memset(parg, 0, n);
		err = v4l2_compat_get_user(arg, parg, cmd);
	} else {
		memset(parg, 0, n);
#if !defined(CONFIG_64BIT) && defined(CONFIG_COMPAT_32BIT_TIME)
		switch (cmd) {
		case VIDIOC_QUERYBUF_TIME32:
		case VIDIOC_QBUF_TIME32:
		case VIDIOC_DQBUF_TIME32:
		case VIDIOC_PREPARE_BUF_TIME32: {
			struct v4l2_buffer_time32 vb32;
			struct v4l2_buffer *vb = parg;

			if (copy_from_user(&vb32, arg, sizeof(vb32)))
				return -EFAULT;

			*vb = (struct v4l2_buffer) {
				.index		= vb32.index,
				.type		= vb32.type,
				.bytesused	= vb32.bytesused,
				.flags		= vb32.flags,
				.field		= vb32.field,
				.timestamp.tv_sec	= vb32.timestamp.tv_sec,
				.timestamp.tv_usec	= vb32.timestamp.tv_usec,
				.timecode	= vb32.timecode,
				.sequence	= vb32.sequence,
				.memory		= vb32.memory,
				.m.userptr	= vb32.m.userptr,
				.length		= vb32.length,
				.request_fd	= vb32.request_fd,
			};
			break;
		}
		}
#endif
	}




	/* zero out anything we don't copy from userspace */
	if (!err && n < _IOC_SIZE(real_cmd))
		memset((u8 *)parg + n, 0, _IOC_SIZE(real_cmd) - n);

	return err;
}



static int check_array_args(unsigned int cmd, void *parg, size_t *array_size,
			    void __user **user_ptr, void ***kernel_ptr)
{
	int ret = 0;

	switch (cmd) {
	case VIDIOC_PREPARE_BUF:
	case VIDIOC_QUERYBUF:
	case VIDIOC_QBUF:
	case VIDIOC_DQBUF: {
		struct v4l2_buffer *buf = parg;

		if (V4L2_TYPE_IS_MULTIPLANAR(buf->type) && buf->length > 0) {
			if (buf->length > VIDEO_MAX_PLANES) {
				ret = -EINVAL;
				break;
			}
			*user_ptr = (void __user *)buf->m.planes;
			*kernel_ptr = (void **)&buf->m.planes;
			*array_size = sizeof(struct v4l2_plane) * buf->length;
			ret = 1;
		}
		break;
	}

	case VIDIOC_G_EDID:
	case VIDIOC_S_EDID: {
		struct v4l2_edid *edid = parg;

		if (edid->blocks) {
			if (edid->blocks > 256) {
				ret = -EINVAL;
				break;
			}
			*user_ptr = (void __user *)edid->edid;
			*kernel_ptr = (void **)&edid->edid;
			*array_size = edid->blocks * 128;
			ret = 1;
		}
		break;
	}

	case VIDIOC_S_EXT_CTRLS:
	case VIDIOC_G_EXT_CTRLS:
	case VIDIOC_TRY_EXT_CTRLS: {
		struct v4l2_ext_controls *ctrls = parg;

		if (ctrls->count != 0) {
			if (ctrls->count > V4L2_CID_MAX_CTRLS) {
				ret = -EINVAL;
				break;
			}
			*user_ptr = (void __user *)ctrls->controls;
			*kernel_ptr = (void **)&ctrls->controls;
			*array_size = sizeof(struct v4l2_ext_control)
				    * ctrls->count;
			ret = 1;
		}
		break;
	}
	case VIDIOC_G_FMT:
	case VIDIOC_S_FMT:
	case VIDIOC_TRY_FMT: {
		struct v4l2_format *fmt = parg;

		if (fmt->type != V4L2_BUF_TYPE_VIDEO_OVERLAY &&
		    fmt->type != V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY)
			break;
		if (fmt->fmt.win.clipcount > 2048)
			return -EINVAL;
		if (!fmt->fmt.win.clipcount)
			break;

		*user_ptr = (void __user *)fmt->fmt.win.clips;
		*kernel_ptr = (void **)&fmt->fmt.win.clips;
		*array_size = sizeof(struct v4l2_clip)
				* fmt->fmt.win.clipcount;

		ret = 1;
		break;
	}
	}

	return ret;
}

static int put_v4l2_format32(struct v4l2_format *p64,
			     struct v4l2_format32 __user *p32)
{
	switch (p64->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		return copy_to_user(&p32->fmt.pix, &p64->fmt.pix,
				    sizeof(p64->fmt.pix)) ? -EFAULT : 0;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		return copy_to_user(&p32->fmt.pix_mp, &p64->fmt.pix_mp,
				    sizeof(p64->fmt.pix_mp)) ? -EFAULT : 0;
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
		return put_v4l2_window32(&p64->fmt.win, &p32->fmt.win);
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
		return copy_to_user(&p32->fmt.vbi, &p64->fmt.vbi,
				    sizeof(p64->fmt.vbi)) ? -EFAULT : 0;
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		return copy_to_user(&p32->fmt.sliced, &p64->fmt.sliced,
				    sizeof(p64->fmt.sliced)) ? -EFAULT : 0;
	case V4L2_BUF_TYPE_SDR_CAPTURE:
	case V4L2_BUF_TYPE_SDR_OUTPUT:
		return copy_to_user(&p32->fmt.sdr, &p64->fmt.sdr,
				    sizeof(p64->fmt.sdr)) ? -EFAULT : 0;
	case V4L2_BUF_TYPE_META_CAPTURE:
	case V4L2_BUF_TYPE_META_OUTPUT:
		return copy_to_user(&p32->fmt.meta, &p64->fmt.meta,
				    sizeof(p64->fmt.meta)) ? -EFAULT : 0;
	default:
		return -EINVAL;
	}
}


static int put_v4l2_framebuffer32(struct v4l2_framebuffer *p64,
				  struct v4l2_framebuffer32 __user *p32)
{
	if (put_user((uintptr_t)p64->base, &p32->base) ||
	    put_user(p64->capability, &p32->capability) ||
	    put_user(p64->flags, &p32->flags) ||
	    copy_to_user(&p32->fmt, &p64->fmt, sizeof(p64->fmt)))
		return -EFAULT;

	return 0;
}


#ifdef CONFIG_COMPAT_32BIT_TIME
static int put_v4l2_buffer32_time32(struct v4l2_buffer *vb,
				    struct v4l2_buffer32_time32 __user *arg)
{
	struct v4l2_buffer32_time32 vb32;

	memset(&vb32, 0, sizeof(vb32));
	vb32 = (struct v4l2_buffer32_time32) {
		.index		= vb->index,
		.type		= vb->type,
		.bytesused	= vb->bytesused,
		.flags		= vb->flags,
		.field		= vb->field,
		.timestamp.tv_sec	= vb->timestamp.tv_sec,
		.timestamp.tv_usec	= vb->timestamp.tv_usec,
		.timecode	= vb->timecode,
		.sequence	= vb->sequence,
		.memory		= vb->memory,
		.m.offset	= vb->m.offset,
		.length		= vb->length,
		.request_fd	= vb->request_fd,
	};
	switch (vb->memory) {
	case V4L2_MEMORY_MMAP:
	case V4L2_MEMORY_OVERLAY:
		vb32.m.offset = vb->m.offset;
		break;
	case V4L2_MEMORY_USERPTR:
		vb32.m.userptr = (uintptr_t)(vb->m.userptr);
		break;
	case V4L2_MEMORY_DMABUF:
		vb32.m.fd = vb->m.fd;
		break;
	}

	if (V4L2_TYPE_IS_MULTIPLANAR(vb->type))
		vb32.m.planes = (uintptr_t)vb->m.planes;

	if (copy_to_user(arg, &vb32, sizeof(vb32)))
		return -EFAULT;

	return 0;
}
#endif


static int put_v4l2_buffer32(struct v4l2_buffer *vb,
			     struct v4l2_buffer32 __user *arg)
{
	struct v4l2_buffer32 vb32;

	memset(&vb32, 0, sizeof(vb32));
	vb32 = (struct v4l2_buffer32) {
		.index		= vb->index,
		.type		= vb->type,
		.bytesused	= vb->bytesused,
		.flags		= vb->flags,
		.field		= vb->field,
		.timestamp.tv_sec	= vb->timestamp.tv_sec,
		.timestamp.tv_usec	= vb->timestamp.tv_usec,
		.timecode	= vb->timecode,
		.sequence	= vb->sequence,
		.memory		= vb->memory,
		.m.offset	= vb->m.offset,
		.length		= vb->length,
		.request_fd	= vb->request_fd,
	};

	switch (vb->memory) {
	case V4L2_MEMORY_MMAP:
	case V4L2_MEMORY_OVERLAY:
		vb32.m.offset = vb->m.offset;
		break;
	case V4L2_MEMORY_USERPTR:
		vb32.m.userptr = (uintptr_t)(vb->m.userptr);
		break;
	case V4L2_MEMORY_DMABUF:
		vb32.m.fd = vb->m.fd;
		break;
	}

	if (V4L2_TYPE_IS_MULTIPLANAR(vb->type))
		vb32.m.planes = (uintptr_t)vb->m.planes;

	if (copy_to_user(arg, &vb32, sizeof(vb32)))
		return -EFAULT;

	return 0;
}


static int put_v4l2_ext_controls32(struct v4l2_ext_controls *p64,
				   struct v4l2_ext_controls32 __user *p32)
{
	struct v4l2_ext_controls32 ec32;

	memset(&ec32, 0, sizeof(ec32));
	ec32 = (struct v4l2_ext_controls32) {
		.which		= p64->which,
		.count		= p64->count,
		.error_idx	= p64->error_idx,
		.request_fd	= p64->request_fd,
		.reserved[0]	= p64->reserved[0],
		.controls	= (uintptr_t)p64->controls,
	};

	if (copy_to_user(p32, &ec32, sizeof(ec32)))
		return -EFAULT;

	return 0;
}


static int put_v4l2_create32(struct v4l2_create_buffers *p64,
			     struct v4l2_create_buffers32 __user *p32)
{
	if (copy_to_user(p32, p64,
			 offsetof(struct v4l2_create_buffers32, format)) ||
	    put_user(p64->capabilities, &p32->capabilities) ||
	    copy_to_user(p32->reserved, p64->reserved, sizeof(p64->reserved)))
		return -EFAULT;
	return put_v4l2_format32(&p64->format, &p32->format);
}



int v4l2_compat_put_user(void __user *arg, void *parg, unsigned int cmd)
{
	switch (cmd) {
	case VIDIOC_G_FMT32:
	case VIDIOC_S_FMT32:
	case VIDIOC_TRY_FMT32:
		return put_v4l2_format32(parg, arg);

	case VIDIOC_G_FBUF32:
		return put_v4l2_framebuffer32(parg, arg);
#ifdef CONFIG_COMPAT_32BIT_TIME
	case VIDIOC_QUERYBUF32_TIME32:
	case VIDIOC_QBUF32_TIME32:
	case VIDIOC_DQBUF32_TIME32:
	case VIDIOC_PREPARE_BUF32_TIME32:
		return put_v4l2_buffer32_time32(parg, arg);
#endif
	case VIDIOC_QUERYBUF32:
	case VIDIOC_QBUF32:
	case VIDIOC_DQBUF32:
	case VIDIOC_PREPARE_BUF32:
		return put_v4l2_buffer32(parg, arg);

	case VIDIOC_G_EXT_CTRLS32:
	case VIDIOC_S_EXT_CTRLS32:
	case VIDIOC_TRY_EXT_CTRLS32:
		return put_v4l2_ext_controls32(parg, arg);

	case VIDIOC_CREATE_BUFS32:
		return put_v4l2_create32(parg, arg);

	case VIDIOC_ENUMSTD32:
		return put_v4l2_standard32(parg, arg);

	case VIDIOC_ENUMINPUT32:
		return put_v4l2_input32(parg, arg);

	case VIDIOC_G_EDID32:
	case VIDIOC_S_EDID32:
		return put_v4l2_edid32(parg, arg);
#ifdef CONFIG_X86_64
	case VIDIOC_DQEVENT32:
		return put_v4l2_event32(parg, arg);
#endif
#ifdef CONFIG_COMPAT_32BIT_TIME
	case VIDIOC_DQEVENT32_TIME32:
		return put_v4l2_event32_time32(parg, arg);
#endif
	}
	return 0;
}


static int video_put_user(void __user *arg, void *parg,
			  unsigned int real_cmd, unsigned int cmd)
{
	if (!(_IOC_DIR(cmd) & _IOC_READ))
		return 0;

	if (cmd == real_cmd) {
		/*  Copy results into user buffer  */
		if (copy_to_user(arg, parg, _IOC_SIZE(cmd)))
			return -EFAULT;
		return 0;
	}

	if (in_compat_syscall())
		return v4l2_compat_put_user(arg, parg, cmd);

#if !defined(CONFIG_64BIT) && defined(CONFIG_COMPAT_32BIT_TIME)
	switch (cmd) {
	case VIDIOC_DQEVENT_TIME32: {
		struct v4l2_event *ev = parg;
		struct v4l2_event_time32 ev32;

		memset(&ev32, 0, sizeof(ev32));

		ev32.type	= ev->type;
		ev32.pending	= ev->pending;
		ev32.sequence	= ev->sequence;
		ev32.timestamp.tv_sec	= ev->timestamp.tv_sec;
		ev32.timestamp.tv_nsec	= ev->timestamp.tv_nsec;
		ev32.id		= ev->id;

		memcpy(&ev32.u, &ev->u, sizeof(ev->u));
		memcpy(&ev32.reserved, &ev->reserved, sizeof(ev->reserved));

		if (copy_to_user(arg, &ev32, sizeof(ev32)))
			return -EFAULT;
		break;
	}
	case VIDIOC_QUERYBUF_TIME32:
	case VIDIOC_QBUF_TIME32:
	case VIDIOC_DQBUF_TIME32:
	case VIDIOC_PREPARE_BUF_TIME32: {
		struct v4l2_buffer *vb = parg;
		struct v4l2_buffer_time32 vb32;

		memset(&vb32, 0, sizeof(vb32));

		vb32.index	= vb->index;
		vb32.type	= vb->type;
		vb32.bytesused	= vb->bytesused;
		vb32.flags	= vb->flags;
		vb32.field	= vb->field;
		vb32.timestamp.tv_sec	= vb->timestamp.tv_sec;
		vb32.timestamp.tv_usec	= vb->timestamp.tv_usec;
		vb32.timecode	= vb->timecode;
		vb32.sequence	= vb->sequence;
		vb32.memory	= vb->memory;
		vb32.m.userptr	= vb->m.userptr;
		vb32.length	= vb->length;
		vb32.request_fd	= vb->request_fd;

		if (copy_to_user(arg, &vb32, sizeof(vb32)))
			return -EFAULT;
		break;
	}
	}
#endif

	return 0;
}

struct v4l2_plane32 {
	__u32			bytesused;
	__u32			length;
	union {
		__u32		mem_offset;
		compat_long_t	userptr;
		__s32		fd;
	} m;
	__u32			data_offset;
	__u32			reserved[11];
};

static int get_v4l2_plane32(struct v4l2_plane *p64,
			    struct v4l2_plane32 __user *p32,
			    enum v4l2_memory memory)
{
	struct v4l2_plane32 plane32;
	typeof(p64->m) m = {};

	if (copy_from_user(&plane32, p32, sizeof(plane32)))
		return -EFAULT;

	switch (memory) {
	case V4L2_MEMORY_MMAP:
	case V4L2_MEMORY_OVERLAY:
		m.mem_offset = plane32.m.mem_offset;
		break;
	case V4L2_MEMORY_USERPTR:
		m.userptr = (unsigned long)compat_ptr(plane32.m.userptr);
		break;
	case V4L2_MEMORY_DMABUF:
		m.fd = plane32.m.fd;
		break;
	}

	memset(p64, 0, sizeof(*p64));
	*p64 = (struct v4l2_plane) {
		.bytesused	= plane32.bytesused,
		.length		= plane32.length,
		.m		= m,
		.data_offset	= plane32.data_offset,
	};

	return 0;
}


static inline bool ctrl_is_pointer(struct file *file, u32 id)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_fh *fh = NULL;
	struct v4l2_ctrl_handler *hdl = NULL;
	struct v4l2_query_ext_ctrl qec = { id };
	const struct v4l2_ioctl_ops *ops = vdev->ioctl_ops;

	if (test_bit(V4L2_FL_USES_V4L2_FH, &vdev->flags))
		fh = file->private_data;

	if (fh && fh->ctrl_handler)
		hdl = fh->ctrl_handler;
	else if (vdev->ctrl_handler)
		hdl = vdev->ctrl_handler;

	if (hdl) {
		struct v4l2_ctrl *ctrl = v4l2_ctrl_find(hdl, id);

		return ctrl && ctrl->is_ptr;
	}

	if (!ops || !ops->vidioc_query_ext_ctrl)
		return false;

	return !ops->vidioc_query_ext_ctrl(file, fh, &qec) &&
		(qec.flags & V4L2_CTRL_FLAG_HAS_PAYLOAD);
}


int v4l2_compat_get_array_args(struct file *file, void *mbuf,
			       void __user *user_ptr, size_t array_size,
			       unsigned int cmd, void *arg)
{
	int err = 0;

	memset(mbuf, 0, array_size);

	switch (cmd) {
	case VIDIOC_G_FMT32:
	case VIDIOC_S_FMT32:
	case VIDIOC_TRY_FMT32: {
		struct v4l2_format *f64 = arg;
		struct v4l2_clip *c64 = mbuf;
		struct v4l2_clip32 __user *c32 = user_ptr;
		u32 clipcount = f64->fmt.win.clipcount;

		if ((f64->type != V4L2_BUF_TYPE_VIDEO_OVERLAY &&
		     f64->type != V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY) ||
		    clipcount == 0)
			return 0;
		if (clipcount > 2048)
			return -EINVAL;
		while (clipcount--) {
			if (copy_from_user(c64, c32, sizeof(c64->c)))
				return -EFAULT;
			c64->next = NULL;
			c64++;
			c32++;
		}
		break;
	}
#ifdef CONFIG_COMPAT_32BIT_TIME
	case VIDIOC_QUERYBUF32_TIME32:
	case VIDIOC_QBUF32_TIME32:
	case VIDIOC_DQBUF32_TIME32:
	case VIDIOC_PREPARE_BUF32_TIME32:
#endif
	case VIDIOC_QUERYBUF32:
	case VIDIOC_QBUF32:
	case VIDIOC_DQBUF32:
	case VIDIOC_PREPARE_BUF32: {
		struct v4l2_buffer *b64 = arg;
		struct v4l2_plane *p64 = mbuf;
		struct v4l2_plane32 __user *p32 = user_ptr;

		if (V4L2_TYPE_IS_MULTIPLANAR(b64->type)) {
			u32 num_planes = b64->length;

			if (num_planes == 0)
				return 0;

			while (num_planes--) {
				err = get_v4l2_plane32(p64, p32, b64->memory);
				if (err)
					return err;
				++p64;
				++p32;
			}
		}
		break;
	}
	case VIDIOC_G_EXT_CTRLS32:
	case VIDIOC_S_EXT_CTRLS32:
	case VIDIOC_TRY_EXT_CTRLS32: {
		struct v4l2_ext_controls *ecs64 = arg;
		struct v4l2_ext_control *ec64 = mbuf;
		struct v4l2_ext_control32 __user *ec32 = user_ptr;
		int n;

		for (n = 0; n < ecs64->count; n++) {
			if (copy_from_user(ec64, ec32, sizeof(*ec32)))
				return -EFAULT;

			if (ctrl_is_pointer(file, ec64->id)) {
				compat_uptr_t p;

				if (get_user(p, &ec32->string))
					return -EFAULT;
				ec64->string = compat_ptr(p);
			}
			ec32++;
			ec64++;
		}
		break;
	}
	default:
		if (copy_from_user(mbuf, user_ptr, array_size))
			err = -EFAULT;
		break;
	}

	return err;
}



static int put_v4l2_plane32(struct v4l2_plane *p64,
			    struct v4l2_plane32 __user *p32,
			    enum v4l2_memory memory)
{
	struct v4l2_plane32 plane32;

	memset(&plane32, 0, sizeof(plane32));
	plane32 = (struct v4l2_plane32) {
		.bytesused	= p64->bytesused,
		.length		= p64->length,
		.data_offset	= p64->data_offset,
	};

	switch (memory) {
	case V4L2_MEMORY_MMAP:
	case V4L2_MEMORY_OVERLAY:
		plane32.m.mem_offset = p64->m.mem_offset;
		break;
	case V4L2_MEMORY_USERPTR:
		plane32.m.userptr = (uintptr_t)(p64->m.userptr);
		break;
	case V4L2_MEMORY_DMABUF:
		plane32.m.fd = p64->m.fd;
		break;
	}

	if (copy_to_user(p32, &plane32, sizeof(plane32)))
		return -EFAULT;

	return 0;
}




int v4l2_compat_put_array_args(struct file *file, void __user *user_ptr,
			       void *mbuf, size_t array_size,
			       unsigned int cmd, void *arg)
{
	int err = 0;

	switch (cmd) {
	case VIDIOC_G_FMT32:
	case VIDIOC_S_FMT32:
	case VIDIOC_TRY_FMT32: {
		struct v4l2_format *f64 = arg;
		struct v4l2_clip *c64 = mbuf;
		struct v4l2_clip32 __user *c32 = user_ptr;
		u32 clipcount = f64->fmt.win.clipcount;

		if ((f64->type != V4L2_BUF_TYPE_VIDEO_OVERLAY &&
		     f64->type != V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY) ||
		    clipcount == 0)
			return 0;
		if (clipcount > 2048)
			return -EINVAL;
		while (clipcount--) {
			if (copy_to_user(c32, c64, sizeof(c64->c)))
				return -EFAULT;
			c64++;
			c32++;
		}
		break;
	}
#ifdef CONFIG_COMPAT_32BIT_TIME
	case VIDIOC_QUERYBUF32_TIME32:
	case VIDIOC_QBUF32_TIME32:
	case VIDIOC_DQBUF32_TIME32:
	case VIDIOC_PREPARE_BUF32_TIME32:
#endif
	case VIDIOC_QUERYBUF32:
	case VIDIOC_QBUF32:
	case VIDIOC_DQBUF32:
	case VIDIOC_PREPARE_BUF32: {
		struct v4l2_buffer *b64 = arg;
		struct v4l2_plane *p64 = mbuf;
		struct v4l2_plane32 __user *p32 = user_ptr;

		if (V4L2_TYPE_IS_MULTIPLANAR(b64->type)) {
			u32 num_planes = b64->length;

			if (num_planes == 0)
				return 0;

			while (num_planes--) {
				err = put_v4l2_plane32(p64, p32, b64->memory);
				if (err)
					return err;
				++p64;
				++p32;
			}
		}
		break;
	}
	case VIDIOC_G_EXT_CTRLS32:
	case VIDIOC_S_EXT_CTRLS32:
	case VIDIOC_TRY_EXT_CTRLS32: {
		struct v4l2_ext_controls *ecs64 = arg;
		struct v4l2_ext_control *ec64 = mbuf;
		struct v4l2_ext_control32 __user *ec32 = user_ptr;
		int n;

		for (n = 0; n < ecs64->count; n++) {
			unsigned int size = sizeof(*ec32);
			/*
			 * Do not modify the pointer when copying a pointer
			 * control.  The contents of the pointer was changed,
			 * not the pointer itself.
			 * The structures are otherwise compatible.
			 */
			if (ctrl_is_pointer(file, ec64->id))
				size -= sizeof(ec32->value64);

			if (copy_to_user(ec32, ec64, size))
				return -EFAULT;

			ec32++;
			ec64++;
		}
		break;
	}
	default:
		if (copy_to_user(user_ptr, mbuf, array_size))
			err = -EFAULT;
		break;
	}

	return err;
}


long
video_usercopy(struct file *file, unsigned int orig_cmd, unsigned long arg,
	       v4l2_kioctl func)
{

	char	sbuf[128];
	void    *mbuf = NULL, *array_buf = NULL;
	void	*parg = (void *)arg;
	long	err  = -EINVAL;
	bool	has_array_args;
	bool	always_copy = false;
	size_t  array_size = 0;
	void __user *user_ptr = NULL;
	void	**kernel_ptr = NULL;
		
	unsigned int cmd = video_translate_cmd(orig_cmd);
	const size_t ioc_size = _IOC_SIZE(cmd);

	printk(KERN_ALERT "video_usercopy : enter!\r\n");


	/*  Copy arguments into temp kernel buffer  */
	if (_IOC_DIR(cmd) != _IOC_NONE) {
		if (ioc_size <= sizeof(sbuf)) {
			parg = sbuf;
		} else {
			/* too big to allocate from stack */
			mbuf = kmalloc(ioc_size, GFP_KERNEL);
			if (NULL == mbuf)
				return -ENOMEM;
			parg = mbuf;
		}

		err = video_get_user((void __user *)arg, parg, cmd,
				     orig_cmd, &always_copy);
		if (err)
			goto out;
	}

	err = check_array_args(cmd, parg, &array_size, &user_ptr, &kernel_ptr);


	if (err < 0)
		goto out;
	has_array_args = err;



	if (has_array_args) {
		array_buf = kvmalloc(array_size, GFP_KERNEL);
		err = -ENOMEM;
		if (array_buf == NULL)
			goto out_array_args;
		err = -EFAULT;
		if (in_compat_syscall())
			err = v4l2_compat_get_array_args(file, array_buf,
							 user_ptr, array_size,
							 orig_cmd, parg);
		else
			err = copy_from_user(array_buf, user_ptr, array_size) ?
								-EFAULT : 0;
		if (err)
			goto out_array_args;
		*kernel_ptr = array_buf;
	}





	/* Handles IOCTL */
	printk(KERN_ALERT "call __video_do_ioctl!\r\n");
	err = func(file, cmd, parg);
	if (err == -ENOTTY || err == -ENOIOCTLCMD) {
		err = -ENOTTY;
		goto out;
	}



	if (err == 0) {
		if (cmd == VIDIOC_DQBUF)
		;
			//trace_v4l2_dqbuf(video_devdata(file)->minor, parg);
		else if (cmd == VIDIOC_QBUF)
		;
			//trace_v4l2_qbuf(video_devdata(file)->minor, parg);
	}

	


	if (has_array_args) {
		*kernel_ptr = (void __force *)user_ptr;
		if (in_compat_syscall()) {
			int put_err;

			put_err = v4l2_compat_put_array_args(file, user_ptr,
							     array_buf,
							     array_size,
							     orig_cmd, parg);
			if (put_err)
				err = put_err;
		} else if (copy_to_user(user_ptr, array_buf, array_size)) {
			err = -EFAULT;
		}
		goto out_array_args;
	}
	/*
	 * Some ioctls can return an error, but still have valid
	 * results that must be returned.
	 */
	if (err < 0 && !always_copy)
		goto out;


out_array_args:
	if (video_put_user((void __user *)arg, parg, cmd, orig_cmd))
		err = -EFAULT;




out:
	kvfree(array_buf);
	kfree(mbuf);


	return err;
}




static void fill_buf_caps(struct vb2_queue *q, u32 *caps)
{
	*caps = V4L2_BUF_CAP_SUPPORTS_ORPHANED_BUFS;
	if (q->io_modes & VB2_MMAP)
		*caps |= V4L2_BUF_CAP_SUPPORTS_MMAP;
	if (q->io_modes & VB2_USERPTR)
		*caps |= V4L2_BUF_CAP_SUPPORTS_USERPTR;
	if (q->io_modes & VB2_DMABUF)
		*caps |= V4L2_BUF_CAP_SUPPORTS_DMABUF;
	if (q->subsystem_flags & VB2_V4L2_FL_SUPPORTS_M2M_HOLD_CAPTURE_BUF)
		*caps |= V4L2_BUF_CAP_SUPPORTS_M2M_HOLD_CAPTURE_BUF;
	if (q->allow_cache_hints && q->io_modes & VB2_MMAP)
		*caps |= V4L2_BUF_CAP_SUPPORTS_MMAP_CACHE_HINTS;
#ifdef CONFIG_MEDIA_CONTROLLER_REQUEST_API
	if (q->supports_requests)
		*caps |= V4L2_BUF_CAP_SUPPORTS_REQUESTS;
#endif
}



int vb2_core_reqbufs(struct vb2_queue *q, enum vb2_memory memory,
		     unsigned int *count)
{
	unsigned int num_buffers, allocated_buffers, num_planes = 0;
	unsigned plane_sizes[VB2_MAX_PLANES] = { };
	unsigned int i;
	int ret;

	if (q->streaming) {
		dprintk(q, 1, "streaming active\n");
		return -EBUSY;
	}

	if (q->waiting_in_dqbuf && *count) {
		dprintk(q, 1, "another dup()ped fd is waiting for a buffer\n");
		return -EBUSY;
	}

	if (*count == 0 || q->num_buffers != 0 ||
	    (q->memory != VB2_MEMORY_UNKNOWN && q->memory != memory)) {
		/*
		 * We already have buffers allocated, so first check if they
		 * are not in use and can be freed.
		 */

		printk(KERN_ALERT "not first\r\n");
		#if 0
		mutex_lock(&q->mmap_lock);
		if (debug && q->memory == VB2_MEMORY_MMAP &&
		    __buffers_in_use(q))
			dprintk(q, 1, "memory in use, orphaning buffers\n");

		/*
		 * Call queue_cancel to clean up any buffers in the
		 * QUEUED state which is possible if buffers were prepared or
		 * queued without ever calling STREAMON.
		 */
		__vb2_queue_cancel(q);
		ret = __vb2_queue_free(q, q->num_buffers);
		mutex_unlock(&q->mmap_lock);
		if (ret)
			return ret;

		/*
		 * In case of REQBUFS(0) return immediately without calling
		 * driver's queue_setup() callback and allocating resources.
		 */
		if (*count == 0)
			return 0;
		
		#endif
	}

	/*
	 * Make sure the requested values and current defaults are sane.
	 */
	WARN_ON(q->min_buffers_needed > VB2_MAX_FRAME);
	num_buffers = max_t(unsigned int, *count, q->min_buffers_needed);
	num_buffers = min_t(unsigned int, num_buffers, VB2_MAX_FRAME);
	memset(q->alloc_devs, 0, sizeof(q->alloc_devs));
	/*
	 * Set this now to ensure that drivers see the correct q->memory value
	 * in the queue_setup op.
	 */
	mutex_lock(&q->mmap_lock);
	q->memory = memory;
	mutex_unlock(&q->mmap_lock);

	/*
	 * Ask the driver how many buffers and planes per buffer it requires.
	 * Driver also sets the size and allocator context for each plane.
	 */
	ret = call_qop(q, queue_setup, q, &num_buffers, &num_planes,
		       plane_sizes, q->alloc_devs);

	//#if 0
	if (ret)
		goto error;

	/* Check that driver has set sane values */
	if (WARN_ON(!num_planes)) {
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < num_planes; i++)
		if (WARN_ON(!plane_sizes[i])) {
			ret = -EINVAL;
			goto error;
		}

	/* Finally, allocate buffers and video memory */
	allocated_buffers =
		__vb2_queue_alloc(q, memory, num_buffers, num_planes, plane_sizes);
	if (allocated_buffers == 0) {
		dprintk(q, 1, "memory allocation failed\n");
		ret = -ENOMEM;
		goto error;
	}

	/*
	 * There is no point in continuing if we can't allocate the minimum
	 * number of buffers needed by this vb2_queue.
	 */
	if (allocated_buffers < q->min_buffers_needed)
		ret = -ENOMEM;

	/*
	 * Check if driver can handle the allocated number of buffers.
	 */
	if (!ret && allocated_buffers < num_buffers) {
		num_buffers = allocated_buffers;
		/*
		 * num_planes is set by the previous queue_setup(), but since it
		 * signals to queue_setup() whether it is called from create_bufs()
		 * vs reqbufs() we zero it here to signal that queue_setup() is
		 * called for the reqbufs() case.
		 */
		num_planes = 0;

		ret = call_qop(q, queue_setup, q, &num_buffers,
			       &num_planes, plane_sizes, q->alloc_devs);

		if (!ret && allocated_buffers < num_buffers)
			ret = -ENOMEM;

		/*
		 * Either the driver has accepted a smaller number of buffers,
		 * or .queue_setup() returned an error
		 */
	}

	mutex_lock(&q->mmap_lock);
	q->num_buffers = allocated_buffers;

	if (ret < 0) {
		/*
		 * Note: __vb2_queue_free() will subtract 'allocated_buffers'
		 * from q->num_buffers and it will reset q->memory to
		 * VB2_MEMORY_UNKNOWN.
		 */
		__vb2_queue_free(q, allocated_buffers);
		mutex_unlock(&q->mmap_lock);
		return ret;
	}
	mutex_unlock(&q->mmap_lock);

	/*
	 * Return the number of successfully allocated buffers
	 * to the userspace.
	 */
	*count = allocated_buffers;
	q->waiting_for_buffers = !q->is_output;

	return 0;

error:
	mutex_lock(&q->mmap_lock);
	q->memory = VB2_MEMORY_UNKNOWN;
	mutex_unlock(&q->mmap_lock);

	//#endif 
	return ret;
}

static inline bool vb2_queue_is_busy(struct video_device *vdev, struct file *file)
{
	return vdev->queue->owner && vdev->queue->owner != file->private_data;
}


int clone_vb2_ioctl_reqbufs(struct file *file, void *priv,
			  struct v4l2_requestbuffers *p)
{
	printk(KERN_ALERT "sample_ioctl_reqbufs : enter!\r\n");
	struct video_device *vdev = video_devdata(file);
	int res = vb2_verify_memory_type(vdev->queue, p->memory, p->type);
	printk(KERN_ALERT "res = %d!\r\n" , res);

	fill_buf_caps(vdev->queue, &p->capabilities);
	if (res)
		return res;
	if (vb2_queue_is_busy(vdev, file))
		return -EBUSY;
	
	printk(KERN_ALERT "res = %d!\r\n" , res);
	res = vb2_core_reqbufs(vdev->queue, p->memory, &p->count);
	/* If count == 0, then the owner has released all buffers and he
	   is no longer owner of the queue. Otherwise we have a new owner. */
	if (res == 0)
		vdev->queue->owner = p->count ? file->private_data : NULL;
	return res;
}





/////////////////////////////////////////////////////////////////////////////////////////
#endif







#if 0
////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * videobuf2-core.c - video buffer 2 core framework
 *
 * Copyright (C) 2010 Samsung Electronics
 *
 * Author: Pawel Osciak <pawel@osciak.com>
 *	   Marek Szyprowski <m.szyprowski@samsung.com>
 *
 * The vb2_thread implementation was based on code from videobuf-dvb.c:
 *	(c) 2004 Gerd Knorr <kraxel@bytesex.org> [SUSE Labs]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 */

//#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

//#define pr_fmt(fmt)

#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/freezer.h>
#include <linux/kthread.h>

#include <media/videobuf2-core.h>
#include <media/v4l2-mc.h>

#include <trace/events/vb2.h>


#define EXPORT_SYMBOL_GPL(X)
//#define MODULE_DESCRIPTION(X)
//#define MODULE_AUTHOR(X);
//#define MODULE_LICENSE(X);
#define EXPORT_SYMBOL(X)

static int debug;
module_param(debug, int, 0644);

#define dprintk(q, level, fmt, arg...)					\
	do {								\
		if (debug >= level)					\
			pr_info("[%s] %s: " fmt, (q)->name, __func__,	\
				## arg);				\
	} while (0)

#ifdef CONFIG_VIDEO_ADV_DEBUG

/*
 * If advanced debugging is on, then count how often each op is called
 * successfully, which can either be per-buffer or per-queue.
 *
 * This makes it easy to check that the 'init' and 'cleanup'
 * (and variations thereof) stay balanced.
 */

#define log_memop(vb, op)						\
	dprintk((vb)->vb2_queue, 2, "call_memop(%d, %s)%s\n",		\
		(vb)->index, #op,					\
		(vb)->vb2_queue->mem_ops->op ? "" : " (nop)")

#define call_memop(vb, op, args...)					\
({									\
	struct vb2_queue *_q = (vb)->vb2_queue;				\
	int err;							\
									\
	log_memop(vb, op);						\
	err = _q->mem_ops->op ? _q->mem_ops->op(args) : 0;		\
	if (!err)							\
		(vb)->cnt_mem_ ## op++;					\
	err;								\
})

#define call_ptr_memop(op, vb, args...)					\
({									\
	struct vb2_queue *_q = (vb)->vb2_queue;				\
	void *ptr;							\
									\
	log_memop(vb, op);						\
	ptr = _q->mem_ops->op ? _q->mem_ops->op(vb, args) : NULL;	\
	if (!IS_ERR_OR_NULL(ptr))					\
		(vb)->cnt_mem_ ## op++;					\
	ptr;								\
})

#define call_void_memop(vb, op, args...)				\
({									\
	struct vb2_queue *_q = (vb)->vb2_queue;				\
									\
	log_memop(vb, op);						\
	if (_q->mem_ops->op)						\
		_q->mem_ops->op(args);					\
	(vb)->cnt_mem_ ## op++;						\
})

#define log_qop(q, op)							\
	dprintk(q, 2, "call_qop(%s)%s\n", #op,				\
		(q)->ops->op ? "" : " (nop)")

#define call_qop(q, op, args...)					\
({									\
	int err;							\
									\
	log_qop(q, op);							\
	err = (q)->ops->op ? (q)->ops->op(args) : 0;			\
	if (!err)							\
		(q)->cnt_ ## op++;					\
	err;								\
})

#define call_void_qop(q, op, args...)					\
({									\
	log_qop(q, op);							\
	if ((q)->ops->op)						\
		(q)->ops->op(args);					\
	(q)->cnt_ ## op++;						\
})

#define log_vb_qop(vb, op, args...)					\
	dprintk((vb)->vb2_queue, 2, "call_vb_qop(%d, %s)%s\n",		\
		(vb)->index, #op,					\
		(vb)->vb2_queue->ops->op ? "" : " (nop)")

#define call_vb_qop(vb, op, args...)					\
({									\
	int err;							\
									\
	log_vb_qop(vb, op);						\
	err = (vb)->vb2_queue->ops->op ?				\
		(vb)->vb2_queue->ops->op(args) : 0;			\
	if (!err)							\
		(vb)->cnt_ ## op++;					\
	err;								\
})

#define call_void_vb_qop(vb, op, args...)				\
({									\
	log_vb_qop(vb, op);						\
	if ((vb)->vb2_queue->ops->op)					\
		(vb)->vb2_queue->ops->op(args);				\
	(vb)->cnt_ ## op++;						\
})

#else

#define call_memop(vb, op, args...)					\
	((vb)->vb2_queue->mem_ops->op ?					\
		(vb)->vb2_queue->mem_ops->op(args) : 0)

#define call_ptr_memop(op, vb, args...)					\
	((vb)->vb2_queue->mem_ops->op ?					\
		(vb)->vb2_queue->mem_ops->op(vb, args) : NULL)

#define call_void_memop(vb, op, args...)				\
	do {								\
		if ((vb)->vb2_queue->mem_ops->op)			\
			(vb)->vb2_queue->mem_ops->op(args);		\
	} while (0)

#define call_qop(q, op, args...)					\
	((q)->ops->op ? (q)->ops->op(args) : 0)

#define call_void_qop(q, op, args...)					\
	do {								\
		if ((q)->ops->op)					\
			(q)->ops->op(args);				\
	} while (0)

#define call_vb_qop(vb, op, args...)					\
	((vb)->vb2_queue->ops->op ? (vb)->vb2_queue->ops->op(args) : 0)

#define call_void_vb_qop(vb, op, args...)				\
	do {								\
		if ((vb)->vb2_queue->ops->op)				\
			(vb)->vb2_queue->ops->op(args);			\
	} while (0)

#endif

#define call_bufop(q, op, args...)					\
({									\
	int ret = 0;							\
	if (q && q->buf_ops && q->buf_ops->op)				\
		ret = q->buf_ops->op(args);				\
	ret;								\
})

#define call_void_bufop(q, op, args...)					\
({									\
	if (q && q->buf_ops && q->buf_ops->op)				\
		q->buf_ops->op(args);					\
})

static void __vb2_queue_cancel(struct vb2_queue *q);
static void __enqueue_in_driver(struct vb2_buffer *vb);

static const char *vb2_state_name(enum vb2_buffer_state s)
{
	static const char * const state_names[] = {
		[VB2_BUF_STATE_DEQUEUED] = "dequeued",
		[VB2_BUF_STATE_IN_REQUEST] = "in request",
		[VB2_BUF_STATE_PREPARING] = "preparing",
		[VB2_BUF_STATE_QUEUED] = "queued",
		[VB2_BUF_STATE_ACTIVE] = "active",
		[VB2_BUF_STATE_DONE] = "done",
		[VB2_BUF_STATE_ERROR] = "error",
	};

	if ((unsigned int)(s) < ARRAY_SIZE(state_names))
		return state_names[s];
	return "unknown";
}

/*
 * __vb2_buf_mem_alloc() - allocate video memory for the given buffer
 */
static int __vb2_buf_mem_alloc(struct vb2_buffer *vb)
{
	struct vb2_queue *q = vb->vb2_queue;
	void *mem_priv;
	int plane;
	int ret = -ENOMEM;

	/*
	 * Allocate memory for all planes in this buffer
	 * NOTE: mmapped areas should be page aligned
	 */
	for (plane = 0; plane < vb->num_planes; ++plane) {
		/* Memops alloc requires size to be page aligned. */
		unsigned long size = PAGE_ALIGN(vb->planes[plane].length);

		/* Did it wrap around? */
		if (size < vb->planes[plane].length)
			goto free;

		mem_priv = call_ptr_memop(alloc,
					  vb,
					  q->alloc_devs[plane] ? : q->dev,
					  size);
		if (IS_ERR_OR_NULL(mem_priv)) {
			if (mem_priv)
				ret = PTR_ERR(mem_priv);
			goto free;
		}

		/* Associate allocator private data with this plane */
		vb->planes[plane].mem_priv = mem_priv;
	}

	return 0;
free:
	/* Free already allocated memory if one of the allocations failed */
	for (; plane > 0; --plane) {
		call_void_memop(vb, put, vb->planes[plane - 1].mem_priv);
		vb->planes[plane - 1].mem_priv = NULL;
	}

	return ret;
}

/*
 * __vb2_buf_mem_free() - free memory of the given buffer
 */
static void __vb2_buf_mem_free(struct vb2_buffer *vb)
{
	unsigned int plane;

	for (plane = 0; plane < vb->num_planes; ++plane) {
		call_void_memop(vb, put, vb->planes[plane].mem_priv);
		vb->planes[plane].mem_priv = NULL;
		dprintk(vb->vb2_queue, 3, "freed plane %d of buffer %d\n",
			plane, vb->index);
	}
}

/*
 * __vb2_buf_userptr_put() - release userspace memory associated with
 * a USERPTR buffer
 */
static void __vb2_buf_userptr_put(struct vb2_buffer *vb)
{
	unsigned int plane;

	for (plane = 0; plane < vb->num_planes; ++plane) {
		if (vb->planes[plane].mem_priv)
			call_void_memop(vb, put_userptr, vb->planes[plane].mem_priv);
		vb->planes[plane].mem_priv = NULL;
	}
}

/*
 * __vb2_plane_dmabuf_put() - release memory associated with
 * a DMABUF shared plane
 */
static void __vb2_plane_dmabuf_put(struct vb2_buffer *vb, struct vb2_plane *p)
{
	if (!p->mem_priv)
		return;

	if (p->dbuf_mapped)
		call_void_memop(vb, unmap_dmabuf, p->mem_priv);

	call_void_memop(vb, detach_dmabuf, p->mem_priv);
	dma_buf_put(p->dbuf);
	p->mem_priv = NULL;
	p->dbuf = NULL;
	p->dbuf_mapped = 0;
}

/*
 * __vb2_buf_dmabuf_put() - release memory associated with
 * a DMABUF shared buffer
 */
static void __vb2_buf_dmabuf_put(struct vb2_buffer *vb)
{
	unsigned int plane;

	for (plane = 0; plane < vb->num_planes; ++plane)
		__vb2_plane_dmabuf_put(vb, &vb->planes[plane]);
}

/*
 * __vb2_buf_mem_prepare() - call ->prepare() on buffer's private memory
 * to sync caches
 */
static void __vb2_buf_mem_prepare(struct vb2_buffer *vb)
{
	unsigned int plane;

	if (vb->synced)
		return;

	if (vb->need_cache_sync_on_prepare) {
		for (plane = 0; plane < vb->num_planes; ++plane)
			call_void_memop(vb, prepare,
					vb->planes[plane].mem_priv);
	}
	vb->synced = 1;
}

/*
 * __vb2_buf_mem_finish() - call ->finish on buffer's private memory
 * to sync caches
 */
static void __vb2_buf_mem_finish(struct vb2_buffer *vb)
{
	unsigned int plane;

	if (!vb->synced)
		return;

	if (vb->need_cache_sync_on_finish) {
		for (plane = 0; plane < vb->num_planes; ++plane)
			call_void_memop(vb, finish,
					vb->planes[plane].mem_priv);
	}
	vb->synced = 0;
}

/*
 * __setup_offsets() - setup unique offsets ("cookies") for every plane in
 * the buffer.
 */
static void __setup_offsets(struct vb2_buffer *vb)
{
	struct vb2_queue *q = vb->vb2_queue;
	unsigned int plane;
	unsigned long off = 0;

	if (vb->index) {
		struct vb2_buffer *prev = q->bufs[vb->index - 1];
		struct vb2_plane *p = &prev->planes[prev->num_planes - 1];

		off = PAGE_ALIGN(p->m.offset + p->length);
	}

	for (plane = 0; plane < vb->num_planes; ++plane) {
		vb->planes[plane].m.offset = off;

		dprintk(q, 3, "buffer %d, plane %d offset 0x%08lx\n",
				vb->index, plane, off);

		off += vb->planes[plane].length;
		off = PAGE_ALIGN(off);
	}
}

/*
 * __vb2_queue_alloc() - allocate videobuf buffer structures and (for MMAP type)
 * video buffer memory for all buffers/planes on the queue and initializes the
 * queue
 *
 * Returns the number of buffers successfully allocated.
 */
static int __vb2_queue_alloc(struct vb2_queue *q, enum vb2_memory memory,
			     unsigned int num_buffers, unsigned int num_planes,
			     const unsigned plane_sizes[VB2_MAX_PLANES])
{
	unsigned int buffer, plane;
	struct vb2_buffer *vb;
	int ret;

	/* Ensure that q->num_buffers+num_buffers is below VB2_MAX_FRAME */
	num_buffers = min_t(unsigned int, num_buffers,
			    VB2_MAX_FRAME - q->num_buffers);

	for (buffer = 0; buffer < num_buffers; ++buffer) {
		/* Allocate videobuf buffer structures */
		vb = kzalloc(q->buf_struct_size, GFP_KERNEL);
		if (!vb) {
			dprintk(q, 1, "memory alloc for buffer struct failed\n");
			break;
		}

		vb->state = VB2_BUF_STATE_DEQUEUED;
		vb->vb2_queue = q;
		vb->num_planes = num_planes;
		vb->index = q->num_buffers + buffer;
		vb->type = q->type;
		vb->memory = memory;
		/*
		 * We need to set these flags here so that the videobuf2 core
		 * will call ->prepare()/->finish() cache sync/flush on vb2
		 * buffers when appropriate. However, we can avoid explicit
		 * ->prepare() and ->finish() cache sync for DMABUF buffers,
		 * because DMA exporter takes care of it.
		 */
		if (q->memory != VB2_MEMORY_DMABUF) {
			vb->need_cache_sync_on_prepare = 1;
			vb->need_cache_sync_on_finish = 1;
		}
		for (plane = 0; plane < num_planes; ++plane) {
			vb->planes[plane].length = plane_sizes[plane];
			vb->planes[plane].min_length = plane_sizes[plane];
		}
		call_void_bufop(q, init_buffer, vb);

		q->bufs[vb->index] = vb;

		/* Allocate video buffer memory for the MMAP type */
		if (memory == VB2_MEMORY_MMAP) {
			ret = __vb2_buf_mem_alloc(vb);
			if (ret) {
				dprintk(q, 1, "failed allocating memory for buffer %d\n",
					buffer);
				q->bufs[vb->index] = NULL;
				kfree(vb);
				break;
			}
			__setup_offsets(vb);
			/*
			 * Call the driver-provided buffer initialization
			 * callback, if given. An error in initialization
			 * results in queue setup failure.
			 */
			ret = call_vb_qop(vb, buf_init, vb);
			if (ret) {
				dprintk(q, 1, "buffer %d %p initialization failed\n",
					buffer, vb);
				__vb2_buf_mem_free(vb);
				q->bufs[vb->index] = NULL;
				kfree(vb);
				break;
			}
		}
	}

	dprintk(q, 3, "allocated %d buffers, %d plane(s) each\n",
		buffer, num_planes);

	return buffer;
}

/*
 * __vb2_free_mem() - release all video buffer memory for a given queue
 */
static void __vb2_free_mem(struct vb2_queue *q, unsigned int buffers)
{
	unsigned int buffer;
	struct vb2_buffer *vb;

	for (buffer = q->num_buffers - buffers; buffer < q->num_buffers;
	     ++buffer) {
		vb = q->bufs[buffer];
		if (!vb)
			continue;

		/* Free MMAP buffers or release USERPTR buffers */
		if (q->memory == VB2_MEMORY_MMAP)
			__vb2_buf_mem_free(vb);
		else if (q->memory == VB2_MEMORY_DMABUF)
			__vb2_buf_dmabuf_put(vb);
		else
			__vb2_buf_userptr_put(vb);
	}
}

/*
 * __vb2_queue_free() - free buffers at the end of the queue - video memory and
 * related information, if no buffers are left return the queue to an
 * uninitialized state. Might be called even if the queue has already been freed.
 */
static int __vb2_queue_free(struct vb2_queue *q, unsigned int buffers)
{
	unsigned int buffer;

	/*
	 * Sanity check: when preparing a buffer the queue lock is released for
	 * a short while (see __buf_prepare for the details), which would allow
	 * a race with a reqbufs which can call this function. Removing the
	 * buffers from underneath __buf_prepare is obviously a bad idea, so we
	 * check if any of the buffers is in the state PREPARING, and if so we
	 * just return -EAGAIN.
	 */
	for (buffer = q->num_buffers - buffers; buffer < q->num_buffers;
	     ++buffer) {
		if (q->bufs[buffer] == NULL)
			continue;
		if (q->bufs[buffer]->state == VB2_BUF_STATE_PREPARING) {
			dprintk(q, 1, "preparing buffers, cannot free\n");
			return -EAGAIN;
		}
	}

	/* Call driver-provided cleanup function for each buffer, if provided */
	for (buffer = q->num_buffers - buffers; buffer < q->num_buffers;
	     ++buffer) {
		struct vb2_buffer *vb = q->bufs[buffer];

		if (vb && vb->planes[0].mem_priv)
			call_void_vb_qop(vb, buf_cleanup, vb);
	}

	/* Release video buffer memory */
	__vb2_free_mem(q, buffers);

#ifdef CONFIG_VIDEO_ADV_DEBUG
	/*
	 * Check that all the calls were balances during the life-time of this
	 * queue. If not (or if the debug level is 1 or up), then dump the
	 * counters to the kernel log.
	 */
	if (q->num_buffers) {
		bool unbalanced = q->cnt_start_streaming != q->cnt_stop_streaming ||
				  q->cnt_wait_prepare != q->cnt_wait_finish;

		if (unbalanced || debug) {
			pr_info("counters for queue %p:%s\n", q,
				unbalanced ? " UNBALANCED!" : "");
			pr_info("     setup: %u start_streaming: %u stop_streaming: %u\n",
				q->cnt_queue_setup, q->cnt_start_streaming,
				q->cnt_stop_streaming);
			pr_info("     wait_prepare: %u wait_finish: %u\n",
				q->cnt_wait_prepare, q->cnt_wait_finish);
		}
		q->cnt_queue_setup = 0;
		q->cnt_wait_prepare = 0;
		q->cnt_wait_finish = 0;
		q->cnt_start_streaming = 0;
		q->cnt_stop_streaming = 0;
	}
	for (buffer = 0; buffer < q->num_buffers; ++buffer) {
		struct vb2_buffer *vb = q->bufs[buffer];
		bool unbalanced = vb->cnt_mem_alloc != vb->cnt_mem_put ||
				  vb->cnt_mem_prepare != vb->cnt_mem_finish ||
				  vb->cnt_mem_get_userptr != vb->cnt_mem_put_userptr ||
				  vb->cnt_mem_attach_dmabuf != vb->cnt_mem_detach_dmabuf ||
				  vb->cnt_mem_map_dmabuf != vb->cnt_mem_unmap_dmabuf ||
				  vb->cnt_buf_queue != vb->cnt_buf_done ||
				  vb->cnt_buf_prepare != vb->cnt_buf_finish ||
				  vb->cnt_buf_init != vb->cnt_buf_cleanup;

		if (unbalanced || debug) {
			pr_info("   counters for queue %p, buffer %d:%s\n",
				q, buffer, unbalanced ? " UNBALANCED!" : "");
			pr_info("     buf_init: %u buf_cleanup: %u buf_prepare: %u buf_finish: %u\n",
				vb->cnt_buf_init, vb->cnt_buf_cleanup,
				vb->cnt_buf_prepare, vb->cnt_buf_finish);
			pr_info("     buf_out_validate: %u buf_queue: %u buf_done: %u buf_request_complete: %u\n",
				vb->cnt_buf_out_validate, vb->cnt_buf_queue,
				vb->cnt_buf_done, vb->cnt_buf_request_complete);
			pr_info("     alloc: %u put: %u prepare: %u finish: %u mmap: %u\n",
				vb->cnt_mem_alloc, vb->cnt_mem_put,
				vb->cnt_mem_prepare, vb->cnt_mem_finish,
				vb->cnt_mem_mmap);
			pr_info("     get_userptr: %u put_userptr: %u\n",
				vb->cnt_mem_get_userptr, vb->cnt_mem_put_userptr);
			pr_info("     attach_dmabuf: %u detach_dmabuf: %u map_dmabuf: %u unmap_dmabuf: %u\n",
				vb->cnt_mem_attach_dmabuf, vb->cnt_mem_detach_dmabuf,
				vb->cnt_mem_map_dmabuf, vb->cnt_mem_unmap_dmabuf);
			pr_info("     get_dmabuf: %u num_users: %u vaddr: %u cookie: %u\n",
				vb->cnt_mem_get_dmabuf,
				vb->cnt_mem_num_users,
				vb->cnt_mem_vaddr,
				vb->cnt_mem_cookie);
		}
	}
#endif

	/* Free videobuf buffers */
	for (buffer = q->num_buffers - buffers; buffer < q->num_buffers;
	     ++buffer) {
		kfree(q->bufs[buffer]);
		q->bufs[buffer] = NULL;
	}

	q->num_buffers -= buffers;
	if (!q->num_buffers) {
		q->memory = VB2_MEMORY_UNKNOWN;
		INIT_LIST_HEAD(&q->queued_list);
	}
	return 0;
}

bool vb2_buffer_in_use(struct vb2_queue *q, struct vb2_buffer *vb)
{
	unsigned int plane;
	for (plane = 0; plane < vb->num_planes; ++plane) {
		void *mem_priv = vb->planes[plane].mem_priv;
		/*
		 * If num_users() has not been provided, call_memop
		 * will return 0, apparently nobody cares about this
		 * case anyway. If num_users() returns more than 1,
		 * we are not the only user of the plane's memory.
		 */
		if (mem_priv && call_memop(vb, num_users, mem_priv) > 1)
			return true;
	}
	return false;
}
EXPORT_SYMBOL(vb2_buffer_in_use);

/*
 * __buffers_in_use() - return true if any buffers on the queue are in use and
 * the queue cannot be freed (by the means of REQBUFS(0)) call
 */
static bool __buffers_in_use(struct vb2_queue *q)
{
	unsigned int buffer;
	for (buffer = 0; buffer < q->num_buffers; ++buffer) {
		if (vb2_buffer_in_use(q, q->bufs[buffer]))
			return true;
	}
	return false;
}

void vb2_core_querybuf(struct vb2_queue *q, unsigned int index, void *pb)
{
	call_void_bufop(q, fill_user_buffer, q->bufs[index], pb);
}


/*
 * __verify_userptr_ops() - verify that all memory operations required for
 * USERPTR queue type have been provided
 */
static int __verify_userptr_ops(struct vb2_queue *q)
{
	if (!(q->io_modes & VB2_USERPTR) || !q->mem_ops->get_userptr ||
	    !q->mem_ops->put_userptr)
		return -EINVAL;

	return 0;
}

/*
 * __verify_mmap_ops() - verify that all memory operations required for
 * MMAP queue type have been provided
 */
static int __verify_mmap_ops(struct vb2_queue *q)
{
	if (!(q->io_modes & VB2_MMAP) || !q->mem_ops->alloc ||
	    !q->mem_ops->put || !q->mem_ops->mmap)
		return -EINVAL;

	return 0;
}

/*
 * __verify_dmabuf_ops() - verify that all memory operations required for
 * DMABUF queue type have been provided
 */
static int __verify_dmabuf_ops(struct vb2_queue *q)
{
	if (!(q->io_modes & VB2_DMABUF) || !q->mem_ops->attach_dmabuf ||
	    !q->mem_ops->detach_dmabuf  || !q->mem_ops->map_dmabuf ||
	    !q->mem_ops->unmap_dmabuf)
		return -EINVAL;

	return 0;
}

int vb2_verify_memory_type(struct vb2_queue *q,
		enum vb2_memory memory, unsigned int type)
{
	if (memory != VB2_MEMORY_MMAP && memory != VB2_MEMORY_USERPTR &&
	    memory != VB2_MEMORY_DMABUF) {
		dprintk(q, 1, "unsupported memory type\n");
		return -EINVAL;
	}

	if (type != q->type) {
		dprintk(q, 1, "requested type is incorrect\n");
		return -EINVAL;
	}

	/*
	 * Make sure all the required memory ops for given memory type
	 * are available.
	 */
	if (memory == VB2_MEMORY_MMAP && __verify_mmap_ops(q)) {
		dprintk(q, 1, "MMAP for current setup unsupported\n");
		return -EINVAL;
	}

	if (memory == VB2_MEMORY_USERPTR && __verify_userptr_ops(q)) {
		dprintk(q, 1, "USERPTR for current setup unsupported\n");
		return -EINVAL;
	}

	if (memory == VB2_MEMORY_DMABUF && __verify_dmabuf_ops(q)) {
		dprintk(q, 1, "DMABUF for current setup unsupported\n");
		return -EINVAL;
	}

	/*
	 * Place the busy tests at the end: -EBUSY can be ignored when
	 * create_bufs is called with count == 0, but count == 0 should still
	 * do the memory and type validation.
	 */
	if (vb2_fileio_is_active(q)) {
		dprintk(q, 1, "file io in progress\n");
		return -EBUSY;
	}
	return 0;
}
EXPORT_SYMBOL(vb2_verify_memory_type);

int vb2_core_reqbufs(struct vb2_queue *q, enum vb2_memory memory,
		     unsigned int *count)
{
	unsigned int num_buffers, allocated_buffers, num_planes = 0;
	unsigned plane_sizes[VB2_MAX_PLANES] = { };
	unsigned int i;
	int ret;

	if (q->streaming) {
		dprintk(q, 1, "streaming active\n");
		return -EBUSY;
	}

	if (q->waiting_in_dqbuf && *count) {
		dprintk(q, 1, "another dup()ped fd is waiting for a buffer\n");
		return -EBUSY;
	}

	if (*count == 0 || q->num_buffers != 0 ||
	    (q->memory != VB2_MEMORY_UNKNOWN && q->memory != memory)) {
		/*
		 * We already have buffers allocated, so first check if they
		 * are not in use and can be freed.
		 */
		mutex_lock(&q->mmap_lock);
		if (debug && q->memory == VB2_MEMORY_MMAP &&
		    __buffers_in_use(q))
			dprintk(q, 1, "memory in use, orphaning buffers\n");

		/*
		 * Call queue_cancel to clean up any buffers in the
		 * QUEUED state which is possible if buffers were prepared or
		 * queued without ever calling STREAMON.
		 */
		__vb2_queue_cancel(q);
		ret = __vb2_queue_free(q, q->num_buffers);
		mutex_unlock(&q->mmap_lock);
		if (ret)
			return ret;

		/*
		 * In case of REQBUFS(0) return immediately without calling
		 * driver's queue_setup() callback and allocating resources.
		 */
		if (*count == 0)
			return 0;
	}

	/*
	 * Make sure the requested values and current defaults are sane.
	 */
	WARN_ON(q->min_buffers_needed > VB2_MAX_FRAME);
	num_buffers = max_t(unsigned int, *count, q->min_buffers_needed);
	num_buffers = min_t(unsigned int, num_buffers, VB2_MAX_FRAME);
	memset(q->alloc_devs, 0, sizeof(q->alloc_devs));
	/*
	 * Set this now to ensure that drivers see the correct q->memory value
	 * in the queue_setup op.
	 */
	mutex_lock(&q->mmap_lock);
	q->memory = memory;
	mutex_unlock(&q->mmap_lock);

	/*
	 * Ask the driver how many buffers and planes per buffer it requires.
	 * Driver also sets the size and allocator context for each plane.
	 */
	ret = call_qop(q, queue_setup, q, &num_buffers, &num_planes,
		       plane_sizes, q->alloc_devs);
	if (ret)
		goto error;

	/* Check that driver has set sane values */
	if (WARN_ON(!num_planes)) {
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < num_planes; i++)
		if (WARN_ON(!plane_sizes[i])) {
			ret = -EINVAL;
			goto error;
		}

	/* Finally, allocate buffers and video memory */
	allocated_buffers =
		__vb2_queue_alloc(q, memory, num_buffers, num_planes, plane_sizes);
	if (allocated_buffers == 0) {
		dprintk(q, 1, "memory allocation failed\n");
		ret = -ENOMEM;
		goto error;
	}

	/*
	 * There is no point in continuing if we can't allocate the minimum
	 * number of buffers needed by this vb2_queue.
	 */
	if (allocated_buffers < q->min_buffers_needed)
		ret = -ENOMEM;

	/*
	 * Check if driver can handle the allocated number of buffers.
	 */
	if (!ret && allocated_buffers < num_buffers) {
		num_buffers = allocated_buffers;
		/*
		 * num_planes is set by the previous queue_setup(), but since it
		 * signals to queue_setup() whether it is called from create_bufs()
		 * vs reqbufs() we zero it here to signal that queue_setup() is
		 * called for the reqbufs() case.
		 */
		num_planes = 0;

		ret = call_qop(q, queue_setup, q, &num_buffers,
			       &num_planes, plane_sizes, q->alloc_devs);

		if (!ret && allocated_buffers < num_buffers)
			ret = -ENOMEM;

		/*
		 * Either the driver has accepted a smaller number of buffers,
		 * or .queue_setup() returned an error
		 */
	}

	mutex_lock(&q->mmap_lock);
	q->num_buffers = allocated_buffers;

	if (ret < 0) {
		/*
		 * Note: __vb2_queue_free() will subtract 'allocated_buffers'
		 * from q->num_buffers and it will reset q->memory to
		 * VB2_MEMORY_UNKNOWN.
		 */
		__vb2_queue_free(q, allocated_buffers);
		mutex_unlock(&q->mmap_lock);
		return ret;
	}
	mutex_unlock(&q->mmap_lock);

	/*
	 * Return the number of successfully allocated buffers
	 * to the userspace.
	 */
	*count = allocated_buffers;
	q->waiting_for_buffers = !q->is_output;

	return 0;

error:
	mutex_lock(&q->mmap_lock);
	q->memory = VB2_MEMORY_UNKNOWN;
	mutex_unlock(&q->mmap_lock);
	return ret;
}


int vb2_core_create_bufs(struct vb2_queue *q, enum vb2_memory memory,
			 unsigned int *count,
			 unsigned int requested_planes,
			 const unsigned int requested_sizes[])
{
	unsigned int num_planes = 0, num_buffers, allocated_buffers;
	unsigned plane_sizes[VB2_MAX_PLANES] = { };
	bool no_previous_buffers = !q->num_buffers;
	int ret;

	if (q->num_buffers == VB2_MAX_FRAME) {
		dprintk(q, 1, "maximum number of buffers already allocated\n");
		return -ENOBUFS;
	}

	if (no_previous_buffers) {
		if (q->waiting_in_dqbuf && *count) {
			dprintk(q, 1, "another dup()ped fd is waiting for a buffer\n");
			return -EBUSY;
		}
		memset(q->alloc_devs, 0, sizeof(q->alloc_devs));
		/*
		 * Set this now to ensure that drivers see the correct q->memory
		 * value in the queue_setup op.
		 */
		mutex_lock(&q->mmap_lock);
		q->memory = memory;
		mutex_unlock(&q->mmap_lock);
		q->waiting_for_buffers = !q->is_output;
	} else {
		if (q->memory != memory) {
			dprintk(q, 1, "memory model mismatch\n");
			return -EINVAL;
		}
	}

	num_buffers = min(*count, VB2_MAX_FRAME - q->num_buffers);

	if (requested_planes && requested_sizes) {
		num_planes = requested_planes;
		memcpy(plane_sizes, requested_sizes, sizeof(plane_sizes));
	}

	/*
	 * Ask the driver, whether the requested number of buffers, planes per
	 * buffer and their sizes are acceptable
	 */
	ret = call_qop(q, queue_setup, q, &num_buffers,
		       &num_planes, plane_sizes, q->alloc_devs);
	if (ret)
		goto error;

	/* Finally, allocate buffers and video memory */
	allocated_buffers = __vb2_queue_alloc(q, memory, num_buffers,
				num_planes, plane_sizes);
	if (allocated_buffers == 0) {
		dprintk(q, 1, "memory allocation failed\n");
		ret = -ENOMEM;
		goto error;
	}

	/*
	 * Check if driver can handle the so far allocated number of buffers.
	 */
	if (allocated_buffers < num_buffers) {
		num_buffers = allocated_buffers;

		/*
		 * q->num_buffers contains the total number of buffers, that the
		 * queue driver has set up
		 */
		ret = call_qop(q, queue_setup, q, &num_buffers,
			       &num_planes, plane_sizes, q->alloc_devs);

		if (!ret && allocated_buffers < num_buffers)
			ret = -ENOMEM;

		/*
		 * Either the driver has accepted a smaller number of buffers,
		 * or .queue_setup() returned an error
		 */
	}

	mutex_lock(&q->mmap_lock);
	q->num_buffers += allocated_buffers;

	if (ret < 0) {
		/*
		 * Note: __vb2_queue_free() will subtract 'allocated_buffers'
		 * from q->num_buffers and it will reset q->memory to
		 * VB2_MEMORY_UNKNOWN.
		 */
		__vb2_queue_free(q, allocated_buffers);
		mutex_unlock(&q->mmap_lock);
		return -ENOMEM;
	}
	mutex_unlock(&q->mmap_lock);

	/*
	 * Return the number of successfully allocated buffers
	 * to the userspace.
	 */
	*count = allocated_buffers;

	return 0;

error:
	if (no_previous_buffers) {
		mutex_lock(&q->mmap_lock);
		q->memory = VB2_MEMORY_UNKNOWN;
		mutex_unlock(&q->mmap_lock);
	}
	return ret;
}


void *vb2_plane_vaddr(struct vb2_buffer *vb, unsigned int plane_no)
{
	if (plane_no >= vb->num_planes || !vb->planes[plane_no].mem_priv)
		return NULL;

	return call_ptr_memop(vaddr, vb, vb->planes[plane_no].mem_priv);

}


void *vb2_plane_cookie(struct vb2_buffer *vb, unsigned int plane_no)
{
	if (plane_no >= vb->num_planes || !vb->planes[plane_no].mem_priv)
		return NULL;

	return call_ptr_memop(cookie, vb, vb->planes[plane_no].mem_priv);
}


void vb2_buffer_done(struct vb2_buffer *vb, enum vb2_buffer_state state)
{
	struct vb2_queue *q = vb->vb2_queue;
	unsigned long flags;

	if (WARN_ON(vb->state != VB2_BUF_STATE_ACTIVE))
		return;

	if (WARN_ON(state != VB2_BUF_STATE_DONE &&
		    state != VB2_BUF_STATE_ERROR &&
		    state != VB2_BUF_STATE_QUEUED))
		state = VB2_BUF_STATE_ERROR;

#ifdef CONFIG_VIDEO_ADV_DEBUG
	/*
	 * Although this is not a callback, it still does have to balance
	 * with the buf_queue op. So update this counter manually.
	 */
	vb->cnt_buf_done++;
#endif
	dprintk(q, 4, "done processing on buffer %d, state: %s\n",
		vb->index, vb2_state_name(state));

	if (state != VB2_BUF_STATE_QUEUED)
		__vb2_buf_mem_finish(vb);

	spin_lock_irqsave(&q->done_lock, flags);
	if (state == VB2_BUF_STATE_QUEUED) {
		vb->state = VB2_BUF_STATE_QUEUED;
	} else {
		/* Add the buffer to the done buffers list */
		list_add_tail(&vb->done_entry, &q->done_list);
		vb->state = state;
	}
	atomic_dec(&q->owned_by_drv_count);

	if (state != VB2_BUF_STATE_QUEUED && vb->req_obj.req) {
		media_request_object_unbind(&vb->req_obj);
		media_request_object_put(&vb->req_obj);
	}

	spin_unlock_irqrestore(&q->done_lock, flags);

	trace_vb2_buf_done(q, vb);

	switch (state) {
	case VB2_BUF_STATE_QUEUED:
		return;
	default:
		/* Inform any processes that may be waiting for buffers */
		wake_up(&q->done_wq);
		break;
	}
}


void vb2_discard_done(struct vb2_queue *q)
{
	struct vb2_buffer *vb;
	unsigned long flags;

	spin_lock_irqsave(&q->done_lock, flags);
	list_for_each_entry(vb, &q->done_list, done_entry)
		vb->state = VB2_BUF_STATE_ERROR;
	spin_unlock_irqrestore(&q->done_lock, flags);
}


/*
 * __prepare_mmap() - prepare an MMAP buffer
 */
static int __prepare_mmap(struct vb2_buffer *vb)
{
	int ret = 0;

	ret = call_bufop(vb->vb2_queue, fill_vb2_buffer,
			 vb, vb->planes);
	return ret ? ret : call_vb_qop(vb, buf_prepare, vb);
}

/*
 * __prepare_userptr() - prepare a USERPTR buffer
 */
static int __prepare_userptr(struct vb2_buffer *vb)
{
	struct vb2_plane planes[VB2_MAX_PLANES];
	struct vb2_queue *q = vb->vb2_queue;
	void *mem_priv;
	unsigned int plane;
	int ret = 0;
	bool reacquired = vb->planes[0].mem_priv == NULL;

	memset(planes, 0, sizeof(planes[0]) * vb->num_planes);
	/* Copy relevant information provided by the userspace */
	ret = call_bufop(vb->vb2_queue, fill_vb2_buffer,
			 vb, planes);
	if (ret)
		return ret;

	for (plane = 0; plane < vb->num_planes; ++plane) {
		/* Skip the plane if already verified */
		if (vb->planes[plane].m.userptr &&
			vb->planes[plane].m.userptr == planes[plane].m.userptr
			&& vb->planes[plane].length == planes[plane].length)
			continue;

		dprintk(q, 3, "userspace address for plane %d changed, reacquiring memory\n",
			plane);

		/* Check if the provided plane buffer is large enough */
		if (planes[plane].length < vb->planes[plane].min_length) {
			dprintk(q, 1, "provided buffer size %u is less than setup size %u for plane %d\n",
						planes[plane].length,
						vb->planes[plane].min_length,
						plane);
			ret = -EINVAL;
			goto err;
		}

		/* Release previously acquired memory if present */
		if (vb->planes[plane].mem_priv) {
			if (!reacquired) {
				reacquired = true;
				vb->copied_timestamp = 0;
				call_void_vb_qop(vb, buf_cleanup, vb);
			}
			call_void_memop(vb, put_userptr, vb->planes[plane].mem_priv);
		}

		vb->planes[plane].mem_priv = NULL;
		vb->planes[plane].bytesused = 0;
		vb->planes[plane].length = 0;
		vb->planes[plane].m.userptr = 0;
		vb->planes[plane].data_offset = 0;

		/* Acquire each plane's memory */
		mem_priv = call_ptr_memop(get_userptr,
					  vb,
					  q->alloc_devs[plane] ? : q->dev,
					  planes[plane].m.userptr,
					  planes[plane].length);
		if (IS_ERR(mem_priv)) {
			dprintk(q, 1, "failed acquiring userspace memory for plane %d\n",
				plane);
			ret = PTR_ERR(mem_priv);
			goto err;
		}
		vb->planes[plane].mem_priv = mem_priv;
	}

	/*
	 * Now that everything is in order, copy relevant information
	 * provided by userspace.
	 */
	for (plane = 0; plane < vb->num_planes; ++plane) {
		vb->planes[plane].bytesused = planes[plane].bytesused;
		vb->planes[plane].length = planes[plane].length;
		vb->planes[plane].m.userptr = planes[plane].m.userptr;
		vb->planes[plane].data_offset = planes[plane].data_offset;
	}

	if (reacquired) {
		/*
		 * One or more planes changed, so we must call buf_init to do
		 * the driver-specific initialization on the newly acquired
		 * buffer, if provided.
		 */
		ret = call_vb_qop(vb, buf_init, vb);
		if (ret) {
			dprintk(q, 1, "buffer initialization failed\n");
			goto err;
		}
	}

	ret = call_vb_qop(vb, buf_prepare, vb);
	if (ret) {
		dprintk(q, 1, "buffer preparation failed\n");
		call_void_vb_qop(vb, buf_cleanup, vb);
		goto err;
	}

	return 0;
err:
	/* In case of errors, release planes that were already acquired */
	for (plane = 0; plane < vb->num_planes; ++plane) {
		if (vb->planes[plane].mem_priv)
			call_void_memop(vb, put_userptr,
				vb->planes[plane].mem_priv);
		vb->planes[plane].mem_priv = NULL;
		vb->planes[plane].m.userptr = 0;
		vb->planes[plane].length = 0;
	}

	return ret;
}

/*
 * __prepare_dmabuf() - prepare a DMABUF buffer
 */
static int __prepare_dmabuf(struct vb2_buffer *vb)
{
	struct vb2_plane planes[VB2_MAX_PLANES];
	struct vb2_queue *q = vb->vb2_queue;
	void *mem_priv;
	unsigned int plane;
	int ret = 0;
	bool reacquired = vb->planes[0].mem_priv == NULL;

	memset(planes, 0, sizeof(planes[0]) * vb->num_planes);
	/* Copy relevant information provided by the userspace */
	ret = call_bufop(vb->vb2_queue, fill_vb2_buffer,
			 vb, planes);
	if (ret)
		return ret;

	for (plane = 0; plane < vb->num_planes; ++plane) {
		struct dma_buf *dbuf = dma_buf_get(planes[plane].m.fd);

		if (IS_ERR_OR_NULL(dbuf)) {
			dprintk(q, 1, "invalid dmabuf fd for plane %d\n",
				plane);
			ret = -EINVAL;
			goto err;
		}

		/* use DMABUF size if length is not provided */
		if (planes[plane].length == 0)
			planes[plane].length = dbuf->size;

		if (planes[plane].length < vb->planes[plane].min_length) {
			dprintk(q, 1, "invalid dmabuf length %u for plane %d, minimum length %u\n",
				planes[plane].length, plane,
				vb->planes[plane].min_length);
			dma_buf_put(dbuf);
			ret = -EINVAL;
			goto err;
		}

		/* Skip the plane if already verified */
		if (dbuf == vb->planes[plane].dbuf &&
			vb->planes[plane].length == planes[plane].length) {
			dma_buf_put(dbuf);
			continue;
		}

		dprintk(q, 3, "buffer for plane %d changed\n", plane);

		if (!reacquired) {
			reacquired = true;
			vb->copied_timestamp = 0;
			call_void_vb_qop(vb, buf_cleanup, vb);
		}

		/* Release previously acquired memory if present */
		__vb2_plane_dmabuf_put(vb, &vb->planes[plane]);
		vb->planes[plane].bytesused = 0;
		vb->planes[plane].length = 0;
		vb->planes[plane].m.fd = 0;
		vb->planes[plane].data_offset = 0;

		/* Acquire each plane's memory */
		mem_priv = call_ptr_memop(attach_dmabuf,
					  vb,
					  q->alloc_devs[plane] ? : q->dev,
					  dbuf,
					  planes[plane].length);
		if (IS_ERR(mem_priv)) {
			dprintk(q, 1, "failed to attach dmabuf\n");
			ret = PTR_ERR(mem_priv);
			dma_buf_put(dbuf);
			goto err;
		}

		vb->planes[plane].dbuf = dbuf;
		vb->planes[plane].mem_priv = mem_priv;
	}

	/*
	 * This pins the buffer(s) with dma_buf_map_attachment()). It's done
	 * here instead just before the DMA, while queueing the buffer(s) so
	 * userspace knows sooner rather than later if the dma-buf map fails.
	 */
	for (plane = 0; plane < vb->num_planes; ++plane) {
		if (vb->planes[plane].dbuf_mapped)
			continue;

		ret = call_memop(vb, map_dmabuf, vb->planes[plane].mem_priv);
		if (ret) {
			dprintk(q, 1, "failed to map dmabuf for plane %d\n",
				plane);
			goto err;
		}
		vb->planes[plane].dbuf_mapped = 1;
	}

	/*
	 * Now that everything is in order, copy relevant information
	 * provided by userspace.
	 */
	for (plane = 0; plane < vb->num_planes; ++plane) {
		vb->planes[plane].bytesused = planes[plane].bytesused;
		vb->planes[plane].length = planes[plane].length;
		vb->planes[plane].m.fd = planes[plane].m.fd;
		vb->planes[plane].data_offset = planes[plane].data_offset;
	}

	if (reacquired) {
		/*
		 * Call driver-specific initialization on the newly acquired buffer,
		 * if provided.
		 */
		ret = call_vb_qop(vb, buf_init, vb);
		if (ret) {
			dprintk(q, 1, "buffer initialization failed\n");
			goto err;
		}
	}

	ret = call_vb_qop(vb, buf_prepare, vb);
	if (ret) {
		dprintk(q, 1, "buffer preparation failed\n");
		call_void_vb_qop(vb, buf_cleanup, vb);
		goto err;
	}

	return 0;
err:
	/* In case of errors, release planes that were already acquired */
	__vb2_buf_dmabuf_put(vb);

	return ret;
}

/*
 * __enqueue_in_driver() - enqueue a vb2_buffer in driver for processing
 */
static void __enqueue_in_driver(struct vb2_buffer *vb)
{
	struct vb2_queue *q = vb->vb2_queue;

	vb->state = VB2_BUF_STATE_ACTIVE;
	atomic_inc(&q->owned_by_drv_count);

	trace_vb2_buf_queue(q, vb);

	call_void_vb_qop(vb, buf_queue, vb);
}

static int __buf_prepare(struct vb2_buffer *vb)
{
	struct vb2_queue *q = vb->vb2_queue;
	enum vb2_buffer_state orig_state = vb->state;
	int ret;

	if (q->error) {
		dprintk(q, 1, "fatal error occurred on queue\n");
		return -EIO;
	}

	if (vb->prepared)
		return 0;
	WARN_ON(vb->synced);

	if (q->is_output) {
		ret = call_vb_qop(vb, buf_out_validate, vb);
		if (ret) {
			dprintk(q, 1, "buffer validation failed\n");
			return ret;
		}
	}

	vb->state = VB2_BUF_STATE_PREPARING;

	switch (q->memory) {
	case VB2_MEMORY_MMAP:
		ret = __prepare_mmap(vb);
		break;
	case VB2_MEMORY_USERPTR:
		ret = __prepare_userptr(vb);
		break;
	case VB2_MEMORY_DMABUF:
		ret = __prepare_dmabuf(vb);
		break;
	default:
		WARN(1, "Invalid queue type\n");
		ret = -EINVAL;
		break;
	}

	if (ret) {
		dprintk(q, 1, "buffer preparation failed: %d\n", ret);
		vb->state = orig_state;
		return ret;
	}

	__vb2_buf_mem_prepare(vb);
	vb->prepared = 1;
	vb->state = orig_state;

	return 0;
}

static int vb2_req_prepare(struct media_request_object *obj)
{
	struct vb2_buffer *vb = container_of(obj, struct vb2_buffer, req_obj);
	int ret;

	if (WARN_ON(vb->state != VB2_BUF_STATE_IN_REQUEST))
		return -EINVAL;

	mutex_lock(vb->vb2_queue->lock);
	ret = __buf_prepare(vb);
	mutex_unlock(vb->vb2_queue->lock);
	return ret;
}

static void __vb2_dqbuf(struct vb2_buffer *vb);

static void vb2_req_unprepare(struct media_request_object *obj)
{
	struct vb2_buffer *vb = container_of(obj, struct vb2_buffer, req_obj);

	mutex_lock(vb->vb2_queue->lock);
	__vb2_dqbuf(vb);
	vb->state = VB2_BUF_STATE_IN_REQUEST;
	mutex_unlock(vb->vb2_queue->lock);
	WARN_ON(!vb->req_obj.req);
}

int vb2_core_qbuf(struct vb2_queue *q, unsigned int index, void *pb,
		  struct media_request *req);

static void vb2_req_queue(struct media_request_object *obj)
{
	struct vb2_buffer *vb = container_of(obj, struct vb2_buffer, req_obj);

	mutex_lock(vb->vb2_queue->lock);
	vb2_core_qbuf(vb->vb2_queue, vb->index, NULL, NULL);
	mutex_unlock(vb->vb2_queue->lock);
}

static void vb2_req_unbind(struct media_request_object *obj)
{
	struct vb2_buffer *vb = container_of(obj, struct vb2_buffer, req_obj);

	if (vb->state == VB2_BUF_STATE_IN_REQUEST)
		call_void_bufop(vb->vb2_queue, init_buffer, vb);
}

static void vb2_req_release(struct media_request_object *obj)
{
	struct vb2_buffer *vb = container_of(obj, struct vb2_buffer, req_obj);

	if (vb->state == VB2_BUF_STATE_IN_REQUEST) {
		vb->state = VB2_BUF_STATE_DEQUEUED;
		if (vb->request)
			media_request_put(vb->request);
		vb->request = NULL;
	}
}

static const struct media_request_object_ops vb2_core_req_ops = {
	.prepare = vb2_req_prepare,
	.unprepare = vb2_req_unprepare,
	.queue = vb2_req_queue,
	.unbind = vb2_req_unbind,
	.release = vb2_req_release,
};

bool vb2_request_object_is_buffer(struct media_request_object *obj)
{
	return obj->ops == &vb2_core_req_ops;
}


unsigned int vb2_request_buffer_cnt(struct media_request *req)
{
	struct media_request_object *obj;
	unsigned long flags;
	unsigned int buffer_cnt = 0;

	spin_lock_irqsave(&req->lock, flags);
	list_for_each_entry(obj, &req->objects, list)
		if (vb2_request_object_is_buffer(obj))
			buffer_cnt++;
	spin_unlock_irqrestore(&req->lock, flags);

	return buffer_cnt;
}


int vb2_core_prepare_buf(struct vb2_queue *q, unsigned int index, void *pb)
{
	struct vb2_buffer *vb;
	int ret;

	vb = q->bufs[index];
	if (vb->state != VB2_BUF_STATE_DEQUEUED) {
		dprintk(q, 1, "invalid buffer state %s\n",
			vb2_state_name(vb->state));
		return -EINVAL;
	}
	if (vb->prepared) {
		dprintk(q, 1, "buffer already prepared\n");
		return -EINVAL;
	}

	ret = __buf_prepare(vb);
	if (ret)
		return ret;

	/* Fill buffer information for the userspace */
	call_void_bufop(q, fill_user_buffer, vb, pb);

	dprintk(q, 2, "prepare of buffer %d succeeded\n", vb->index);

	return 0;
}


/*
 * vb2_start_streaming() - Attempt to start streaming.
 * @q:		videobuf2 queue
 *
 * Attempt to start streaming. When this function is called there must be
 * at least q->min_buffers_needed buffers queued up (i.e. the minimum
 * number of buffers required for the DMA engine to function). If the
 * @start_streaming op fails it is supposed to return all the driver-owned
 * buffers back to vb2 in state QUEUED. Check if that happened and if
 * not warn and reclaim them forcefully.
 */
static int vb2_start_streaming(struct vb2_queue *q)
{
	struct vb2_buffer *vb;
	int ret;

	/*
	 * If any buffers were queued before streamon,
	 * we can now pass them to driver for processing.
	 */
	list_for_each_entry(vb, &q->queued_list, queued_entry)
		__enqueue_in_driver(vb);

	/* Tell the driver to start streaming */
	q->start_streaming_called = 1;
	ret = call_qop(q, start_streaming, q,
		       atomic_read(&q->owned_by_drv_count));
	if (!ret)
		return 0;

	q->start_streaming_called = 0;

	dprintk(q, 1, "driver refused to start streaming\n");
	/*
	 * If you see this warning, then the driver isn't cleaning up properly
	 * after a failed start_streaming(). See the start_streaming()
	 * documentation in videobuf2-core.h for more information how buffers
	 * should be returned to vb2 in start_streaming().
	 */
	if (WARN_ON(atomic_read(&q->owned_by_drv_count))) {
		unsigned i;

		/*
		 * Forcefully reclaim buffers if the driver did not
		 * correctly return them to vb2.
		 */
		for (i = 0; i < q->num_buffers; ++i) {
			vb = q->bufs[i];
			if (vb->state == VB2_BUF_STATE_ACTIVE)
				vb2_buffer_done(vb, VB2_BUF_STATE_QUEUED);
		}
		/* Must be zero now */
		WARN_ON(atomic_read(&q->owned_by_drv_count));
	}
	/*
	 * If done_list is not empty, then start_streaming() didn't call
	 * vb2_buffer_done(vb, VB2_BUF_STATE_QUEUED) but STATE_ERROR or
	 * STATE_DONE.
	 */
	WARN_ON(!list_empty(&q->done_list));
	return ret;
}

int vb2_core_qbuf(struct vb2_queue *q, unsigned int index, void *pb,
		  struct media_request *req)
{
	struct vb2_buffer *vb;
	enum vb2_buffer_state orig_state;
	int ret;

	if (q->error) {
		dprintk(q, 1, "fatal error occurred on queue\n");
		return -EIO;
	}

	vb = q->bufs[index];

	if (!req && vb->state != VB2_BUF_STATE_IN_REQUEST &&
	    q->requires_requests) {
		dprintk(q, 1, "qbuf requires a request\n");
		return -EBADR;
	}

	if ((req && q->uses_qbuf) ||
	    (!req && vb->state != VB2_BUF_STATE_IN_REQUEST &&
	     q->uses_requests)) {
		dprintk(q, 1, "queue in wrong mode (qbuf vs requests)\n");
		return -EBUSY;
	}

	if (req) {
		int ret;

		q->uses_requests = 1;
		if (vb->state != VB2_BUF_STATE_DEQUEUED) {
			dprintk(q, 1, "buffer %d not in dequeued state\n",
				vb->index);
			return -EINVAL;
		}

		if (q->is_output && !vb->prepared) {
			ret = call_vb_qop(vb, buf_out_validate, vb);
			if (ret) {
				dprintk(q, 1, "buffer validation failed\n");
				return ret;
			}
		}

		media_request_object_init(&vb->req_obj);

		/* Make sure the request is in a safe state for updating. */
		ret = media_request_lock_for_update(req);
		if (ret)
			return ret;
		ret = media_request_object_bind(req, &vb2_core_req_ops,
						q, true, &vb->req_obj);
		media_request_unlock_for_update(req);
		if (ret)
			return ret;

		vb->state = VB2_BUF_STATE_IN_REQUEST;

		/*
		 * Increment the refcount and store the request.
		 * The request refcount is decremented again when the
		 * buffer is dequeued. This is to prevent vb2_buffer_done()
		 * from freeing the request from interrupt context, which can
		 * happen if the application closed the request fd after
		 * queueing the request.
		 */
		media_request_get(req);
		vb->request = req;

		/* Fill buffer information for the userspace */
		if (pb) {
			call_void_bufop(q, copy_timestamp, vb, pb);
			call_void_bufop(q, fill_user_buffer, vb, pb);
		}

		dprintk(q, 2, "qbuf of buffer %d succeeded\n", vb->index);
		return 0;
	}

	if (vb->state != VB2_BUF_STATE_IN_REQUEST)
		q->uses_qbuf = 1;

	switch (vb->state) {
	case VB2_BUF_STATE_DEQUEUED:
	case VB2_BUF_STATE_IN_REQUEST:
		if (!vb->prepared) {
			ret = __buf_prepare(vb);
			if (ret)
				return ret;
		}
		break;
	case VB2_BUF_STATE_PREPARING:
		dprintk(q, 1, "buffer still being prepared\n");
		return -EINVAL;
	default:
		dprintk(q, 1, "invalid buffer state %s\n",
			vb2_state_name(vb->state));
		return -EINVAL;
	}

	/*
	 * Add to the queued buffers list, a buffer will stay on it until
	 * dequeued in dqbuf.
	 */
	orig_state = vb->state;
	list_add_tail(&vb->queued_entry, &q->queued_list);
	q->queued_count++;
	q->waiting_for_buffers = false;
	vb->state = VB2_BUF_STATE_QUEUED;

	if (pb)
		call_void_bufop(q, copy_timestamp, vb, pb);

	trace_vb2_qbuf(q, vb);

	/*
	 * If already streaming, give the buffer to driver for processing.
	 * If not, the buffer will be given to driver on next streamon.
	 */
	if (q->start_streaming_called)
		__enqueue_in_driver(vb);

	/* Fill buffer information for the userspace */
	if (pb)
		call_void_bufop(q, fill_user_buffer, vb, pb);

	/*
	 * If streamon has been called, and we haven't yet called
	 * start_streaming() since not enough buffers were queued, and
	 * we now have reached the minimum number of queued buffers,
	 * then we can finally call start_streaming().
	 */
	if (q->streaming && !q->start_streaming_called &&
	    q->queued_count >= q->min_buffers_needed) {
		ret = vb2_start_streaming(q);
		if (ret) {
			/*
			 * Since vb2_core_qbuf will return with an error,
			 * we should return it to state DEQUEUED since
			 * the error indicates that the buffer wasn't queued.
			 */
			list_del(&vb->queued_entry);
			q->queued_count--;
			vb->state = orig_state;
			return ret;
		}
	}

	dprintk(q, 2, "qbuf of buffer %d succeeded\n", vb->index);
	return 0;
}


/*
 * __vb2_wait_for_done_vb() - wait for a buffer to become available
 * for dequeuing
 *
 * Will sleep if required for nonblocking == false.
 */
static int __vb2_wait_for_done_vb(struct vb2_queue *q, int nonblocking)
{
	/*
	 * All operations on vb_done_list are performed under done_lock
	 * spinlock protection. However, buffers may be removed from
	 * it and returned to userspace only while holding both driver's
	 * lock and the done_lock spinlock. Thus we can be sure that as
	 * long as we hold the driver's lock, the list will remain not
	 * empty if list_empty() check succeeds.
	 */

	for (;;) {
		int ret;

		if (q->waiting_in_dqbuf) {
			dprintk(q, 1, "another dup()ped fd is waiting for a buffer\n");
			return -EBUSY;
		}

		if (!q->streaming) {
			dprintk(q, 1, "streaming off, will not wait for buffers\n");
			return -EINVAL;
		}

		if (q->error) {
			dprintk(q, 1, "Queue in error state, will not wait for buffers\n");
			return -EIO;
		}

		if (q->last_buffer_dequeued) {
			dprintk(q, 3, "last buffer dequeued already, will not wait for buffers\n");
			return -EPIPE;
		}

		if (!list_empty(&q->done_list)) {
			/*
			 * Found a buffer that we were waiting for.
			 */
			break;
		}

		if (nonblocking) {
			dprintk(q, 3, "nonblocking and no buffers to dequeue, will not wait\n");
			return -EAGAIN;
		}

		q->waiting_in_dqbuf = 1;
		/*
		 * We are streaming and blocking, wait for another buffer to
		 * become ready or for streamoff. Driver's lock is released to
		 * allow streamoff or qbuf to be called while waiting.
		 */
		call_void_qop(q, wait_prepare, q);

		/*
		 * All locks have been released, it is safe to sleep now.
		 */
		dprintk(q, 3, "will sleep waiting for buffers\n");
		ret = wait_event_interruptible(q->done_wq,
				!list_empty(&q->done_list) || !q->streaming ||
				q->error);

		/*
		 * We need to reevaluate both conditions again after reacquiring
		 * the locks or return an error if one occurred.
		 */
		call_void_qop(q, wait_finish, q);
		q->waiting_in_dqbuf = 0;
		if (ret) {
			dprintk(q, 1, "sleep was interrupted\n");
			return ret;
		}
	}
	return 0;
}

/*
 * __vb2_get_done_vb() - get a buffer ready for dequeuing
 *
 * Will sleep if required for nonblocking == false.
 */
static int __vb2_get_done_vb(struct vb2_queue *q, struct vb2_buffer **vb,
			     void *pb, int nonblocking)
{
	unsigned long flags;
	int ret = 0;

	/*
	 * Wait for at least one buffer to become available on the done_list.
	 */
	ret = __vb2_wait_for_done_vb(q, nonblocking);
	if (ret)
		return ret;

	/*
	 * Driver's lock has been held since we last verified that done_list
	 * is not empty, so no need for another list_empty(done_list) check.
	 */
	spin_lock_irqsave(&q->done_lock, flags);
	*vb = list_first_entry(&q->done_list, struct vb2_buffer, done_entry);
	/*
	 * Only remove the buffer from done_list if all planes can be
	 * handled. Some cases such as V4L2 file I/O and DVB have pb
	 * == NULL; skip the check then as there's nothing to verify.
	 */
	if (pb)
		ret = call_bufop(q, verify_planes_array, *vb, pb);
	if (!ret)
		list_del(&(*vb)->done_entry);
	spin_unlock_irqrestore(&q->done_lock, flags);

	return ret;
}

int vb2_wait_for_all_buffers(struct vb2_queue *q)
{
	if (!q->streaming) {
		dprintk(q, 1, "streaming off, will not wait for buffers\n");
		return -EINVAL;
	}

	if (q->start_streaming_called)
		wait_event(q->done_wq, !atomic_read(&q->owned_by_drv_count));
	return 0;
}


/*
 * __vb2_dqbuf() - bring back the buffer to the DEQUEUED state
 */
static void __vb2_dqbuf(struct vb2_buffer *vb)
{
	struct vb2_queue *q = vb->vb2_queue;

	/* nothing to do if the buffer is already dequeued */
	if (vb->state == VB2_BUF_STATE_DEQUEUED)
		return;

	vb->state = VB2_BUF_STATE_DEQUEUED;

	call_void_bufop(q, init_buffer, vb);
}

int vb2_core_dqbuf(struct vb2_queue *q, unsigned int *pindex, void *pb,
		   bool nonblocking)
{
	struct vb2_buffer *vb = NULL;
	int ret;

	ret = __vb2_get_done_vb(q, &vb, pb, nonblocking);
	if (ret < 0)
		return ret;

	switch (vb->state) {
	case VB2_BUF_STATE_DONE:
		dprintk(q, 3, "returning done buffer\n");
		break;
	case VB2_BUF_STATE_ERROR:
		dprintk(q, 3, "returning done buffer with errors\n");
		break;
	default:
		dprintk(q, 1, "invalid buffer state %s\n",
			vb2_state_name(vb->state));
		return -EINVAL;
	}

	call_void_vb_qop(vb, buf_finish, vb);
	vb->prepared = 0;

	if (pindex)
		*pindex = vb->index;

	/* Fill buffer information for the userspace */
	if (pb)
		call_void_bufop(q, fill_user_buffer, vb, pb);

	/* Remove from videobuf queue */
	list_del(&vb->queued_entry);
	q->queued_count--;

	trace_vb2_dqbuf(q, vb);

	/* go back to dequeued state */
	__vb2_dqbuf(vb);

	if (WARN_ON(vb->req_obj.req)) {
		media_request_object_unbind(&vb->req_obj);
		media_request_object_put(&vb->req_obj);
	}
	if (vb->request)
		media_request_put(vb->request);
	vb->request = NULL;

	dprintk(q, 2, "dqbuf of buffer %d, state: %s\n",
		vb->index, vb2_state_name(vb->state));

	return 0;

}


/*
 * __vb2_queue_cancel() - cancel and stop (pause) streaming
 *
 * Removes all queued buffers from driver's queue and all buffers queued by
 * userspace from videobuf's queue. Returns to state after reqbufs.
 */
static void __vb2_queue_cancel(struct vb2_queue *q)
{
	unsigned int i;

	/*
	 * Tell driver to stop all transactions and release all queued
	 * buffers.
	 */
	if (q->start_streaming_called)
		call_void_qop(q, stop_streaming, q);

	/*
	 * If you see this warning, then the driver isn't cleaning up properly
	 * in stop_streaming(). See the stop_streaming() documentation in
	 * videobuf2-core.h for more information how buffers should be returned
	 * to vb2 in stop_streaming().
	 */
	if (WARN_ON(atomic_read(&q->owned_by_drv_count))) {
		for (i = 0; i < q->num_buffers; ++i)
			if (q->bufs[i]->state == VB2_BUF_STATE_ACTIVE) {
				pr_warn("driver bug: stop_streaming operation is leaving buf %p in active state\n",
					q->bufs[i]);
				vb2_buffer_done(q->bufs[i], VB2_BUF_STATE_ERROR);
			}
		/* Must be zero now */
		WARN_ON(atomic_read(&q->owned_by_drv_count));
	}

	q->streaming = 0;
	q->start_streaming_called = 0;
	q->queued_count = 0;
	q->error = 0;
	q->uses_requests = 0;
	q->uses_qbuf = 0;

	/*
	 * Remove all buffers from videobuf's list...
	 */
	INIT_LIST_HEAD(&q->queued_list);
	/*
	 * ...and done list; userspace will not receive any buffers it
	 * has not already dequeued before initiating cancel.
	 */
	INIT_LIST_HEAD(&q->done_list);
	atomic_set(&q->owned_by_drv_count, 0);
	wake_up_all(&q->done_wq);

	/*
	 * Reinitialize all buffers for next use.
	 * Make sure to call buf_finish for any queued buffers. Normally
	 * that's done in dqbuf, but that's not going to happen when we
	 * cancel the whole queue. Note: this code belongs here, not in
	 * __vb2_dqbuf() since in vb2_core_dqbuf() there is a critical
	 * call to __fill_user_buffer() after buf_finish(). That order can't
	 * be changed, so we can't move the buf_finish() to __vb2_dqbuf().
	 */
	for (i = 0; i < q->num_buffers; ++i) {
		struct vb2_buffer *vb = q->bufs[i];
		struct media_request *req = vb->req_obj.req;

		/*
		 * If a request is associated with this buffer, then
		 * call buf_request_cancel() to give the driver to complete()
		 * related request objects. Otherwise those objects would
		 * never complete.
		 */
		if (req) {
			enum media_request_state state;
			unsigned long flags;

			spin_lock_irqsave(&req->lock, flags);
			state = req->state;
			spin_unlock_irqrestore(&req->lock, flags);

			if (state == MEDIA_REQUEST_STATE_QUEUED)
				call_void_vb_qop(vb, buf_request_complete, vb);
		}

		__vb2_buf_mem_finish(vb);

		if (vb->prepared) {
			call_void_vb_qop(vb, buf_finish, vb);
			vb->prepared = 0;
		}
		__vb2_dqbuf(vb);

		if (vb->req_obj.req) {
			media_request_object_unbind(&vb->req_obj);
			media_request_object_put(&vb->req_obj);
		}
		if (vb->request)
			media_request_put(vb->request);
		vb->request = NULL;
		vb->copied_timestamp = 0;
	}
}

int vb2_core_streamon(struct vb2_queue *q, unsigned int type)
{
	int ret;

	if (type != q->type) {
		dprintk(q, 1, "invalid stream type\n");
		return -EINVAL;
	}

	if (q->streaming) {
		dprintk(q, 3, "already streaming\n");
		return 0;
	}

	if (!q->num_buffers) {
		dprintk(q, 1, "no buffers have been allocated\n");
		return -EINVAL;
	}

	if (q->num_buffers < q->min_buffers_needed) {
		dprintk(q, 1, "need at least %u allocated buffers\n",
				q->min_buffers_needed);
		return -EINVAL;
	}

	/*
	 * Tell driver to start streaming provided sufficient buffers
	 * are available.
	 */
	if (q->queued_count >= q->min_buffers_needed) {
		ret = v4l_vb2q_enable_media_source(q);
		if (ret)
			return ret;
		ret = vb2_start_streaming(q);
		if (ret)
			return ret;
	}

	q->streaming = 1;

	dprintk(q, 3, "successful\n");
	return 0;
}


void vb2_queue_error(struct vb2_queue *q)
{
	q->error = 1;

	wake_up_all(&q->done_wq);
}


int vb2_core_streamoff(struct vb2_queue *q, unsigned int type)
{
	if (type != q->type) {
		dprintk(q, 1, "invalid stream type\n");
		return -EINVAL;
	}

	/*
	 * Cancel will pause streaming and remove all buffers from the driver
	 * and videobuf, effectively returning control over them to userspace.
	 *
	 * Note that we do this even if q->streaming == 0: if you prepare or
	 * queue buffers, and then call streamoff without ever having called
	 * streamon, you would still expect those buffers to be returned to
	 * their normal dequeued state.
	 */
	__vb2_queue_cancel(q);
	q->waiting_for_buffers = !q->is_output;
	q->last_buffer_dequeued = false;

	dprintk(q, 3, "successful\n");
	return 0;
}


/*
 * __find_plane_by_offset() - find plane associated with the given offset off
 */
static int __find_plane_by_offset(struct vb2_queue *q, unsigned long off,
			unsigned int *_buffer, unsigned int *_plane)
{
	struct vb2_buffer *vb;
	unsigned int buffer, plane;

	/*
	 * Sanity checks to ensure the lock is held, MEMORY_MMAP is
	 * used and fileio isn't active.
	 */
	lockdep_assert_held(&q->mmap_lock);

	if (q->memory != VB2_MEMORY_MMAP) {
		dprintk(q, 1, "queue is not currently set up for mmap\n");
		return -EINVAL;
	}

	if (vb2_fileio_is_active(q)) {
		dprintk(q, 1, "file io in progress\n");
		return -EBUSY;
	}

	/*
	 * Go over all buffers and their planes, comparing the given offset
	 * with an offset assigned to each plane. If a match is found,
	 * return its buffer and plane numbers.
	 */
	for (buffer = 0; buffer < q->num_buffers; ++buffer) {
		vb = q->bufs[buffer];

		for (plane = 0; plane < vb->num_planes; ++plane) {
			if (vb->planes[plane].m.offset == off) {
				*_buffer = buffer;
				*_plane = plane;
				return 0;
			}
		}
	}

	return -EINVAL;
}

int vb2_core_expbuf(struct vb2_queue *q, int *fd, unsigned int type,
		unsigned int index, unsigned int plane, unsigned int flags)
{
	struct vb2_buffer *vb = NULL;
	struct vb2_plane *vb_plane;
	int ret;
	struct dma_buf *dbuf;

	if (q->memory != VB2_MEMORY_MMAP) {
		dprintk(q, 1, "queue is not currently set up for mmap\n");
		return -EINVAL;
	}

	if (!q->mem_ops->get_dmabuf) {
		dprintk(q, 1, "queue does not support DMA buffer exporting\n");
		return -EINVAL;
	}

	if (flags & ~(O_CLOEXEC | O_ACCMODE)) {
		dprintk(q, 1, "queue does support only O_CLOEXEC and access mode flags\n");
		return -EINVAL;
	}

	if (type != q->type) {
		dprintk(q, 1, "invalid buffer type\n");
		return -EINVAL;
	}

	if (index >= q->num_buffers) {
		dprintk(q, 1, "buffer index out of range\n");
		return -EINVAL;
	}

	vb = q->bufs[index];

	if (plane >= vb->num_planes) {
		dprintk(q, 1, "buffer plane out of range\n");
		return -EINVAL;
	}

	if (vb2_fileio_is_active(q)) {
		dprintk(q, 1, "expbuf: file io in progress\n");
		return -EBUSY;
	}

	vb_plane = &vb->planes[plane];

	dbuf = call_ptr_memop(get_dmabuf,
			      vb,
			      vb_plane->mem_priv,
			      flags & O_ACCMODE);
	if (IS_ERR_OR_NULL(dbuf)) {
		dprintk(q, 1, "failed to export buffer %d, plane %d\n",
			index, plane);
		return -EINVAL;
	}

	ret = dma_buf_fd(dbuf, flags & ~O_ACCMODE);
	if (ret < 0) {
		dprintk(q, 3, "buffer %d, plane %d failed to export (%d)\n",
			index, plane, ret);
		dma_buf_put(dbuf);
		return ret;
	}

	dprintk(q, 3, "buffer %d, plane %d exported as %d descriptor\n",
		index, plane, ret);
	*fd = ret;

	return 0;
}


int vb2_mmap(struct vb2_queue *q, struct vm_area_struct *vma)
{
	unsigned long off = vma->vm_pgoff << PAGE_SHIFT;
	struct vb2_buffer *vb;
	unsigned int buffer = 0, plane = 0;
	int ret;
	unsigned long length;

	/*
	 * Check memory area access mode.
	 */
	if (!(vma->vm_flags & VM_SHARED)) {
		dprintk(q, 1, "invalid vma flags, VM_SHARED needed\n");
		return -EINVAL;
	}
	if (q->is_output) {
		if (!(vma->vm_flags & VM_WRITE)) {
			dprintk(q, 1, "invalid vma flags, VM_WRITE needed\n");
			return -EINVAL;
		}
	} else {
		if (!(vma->vm_flags & VM_READ)) {
			dprintk(q, 1, "invalid vma flags, VM_READ needed\n");
			return -EINVAL;
		}
	}

	mutex_lock(&q->mmap_lock);

	/*
	 * Find the plane corresponding to the offset passed by userspace. This
	 * will return an error if not MEMORY_MMAP or file I/O is in progress.
	 */
	ret = __find_plane_by_offset(q, off, &buffer, &plane);
	if (ret)
		goto unlock;

	vb = q->bufs[buffer];

	/*
	 * MMAP requires page_aligned buffers.
	 * The buffer length was page_aligned at __vb2_buf_mem_alloc(),
	 * so, we need to do the same here.
	 */
	length = PAGE_ALIGN(vb->planes[plane].length);
	if (length < (vma->vm_end - vma->vm_start)) {
		dprintk(q, 1,
			"MMAP invalid, as it would overflow buffer length\n");
		ret = -EINVAL;
		goto unlock;
	}

	/*
	 * vm_pgoff is treated in V4L2 API as a 'cookie' to select a buffer,
	 * not as a in-buffer offset. We always want to mmap a whole buffer
	 * from its beginning.
	 */
	vma->vm_pgoff = 0;

	ret = call_memop(vb, mmap, vb->planes[plane].mem_priv, vma);

unlock:
	mutex_unlock(&q->mmap_lock);
	if (ret)
		return ret;

	dprintk(q, 3, "buffer %d, plane %d successfully mapped\n", buffer, plane);
	return 0;
}


#ifndef CONFIG_MMU
unsigned long vb2_get_unmapped_area(struct vb2_queue *q,
				    unsigned long addr,
				    unsigned long len,
				    unsigned long pgoff,
				    unsigned long flags)
{
	unsigned long off = pgoff << PAGE_SHIFT;
	struct vb2_buffer *vb;
	unsigned int buffer, plane;
	void *vaddr;
	int ret;

	mutex_lock(&q->mmap_lock);

	/*
	 * Find the plane corresponding to the offset passed by userspace. This
	 * will return an error if not MEMORY_MMAP or file I/O is in progress.
	 */
	ret = __find_plane_by_offset(q, off, &buffer, &plane);
	if (ret)
		goto unlock;

	vb = q->bufs[buffer];

	vaddr = vb2_plane_vaddr(vb, plane);
	mutex_unlock(&q->mmap_lock);
	return vaddr ? (unsigned long)vaddr : -EINVAL;

unlock:
	mutex_unlock(&q->mmap_lock);
	return ret;
}

#endif

int vb2_core_queue_init(struct vb2_queue *q)
{
	/*
	 * Sanity check
	 */
	if (WARN_ON(!q)			  ||
	    WARN_ON(!q->ops)		  ||
	    WARN_ON(!q->mem_ops)	  ||
	    WARN_ON(!q->type)		  ||
	    WARN_ON(!q->io_modes)	  ||
	    WARN_ON(!q->ops->queue_setup) ||
	    WARN_ON(!q->ops->buf_queue))
		return -EINVAL;

	if (WARN_ON(q->requires_requests && !q->supports_requests))
		return -EINVAL;

	INIT_LIST_HEAD(&q->queued_list);
	INIT_LIST_HEAD(&q->done_list);
	spin_lock_init(&q->done_lock);
	mutex_init(&q->mmap_lock);
	init_waitqueue_head(&q->done_wq);

	q->memory = VB2_MEMORY_UNKNOWN;

	if (q->buf_struct_size == 0)
		q->buf_struct_size = sizeof(struct vb2_buffer);

	if (q->bidirectional)
		q->dma_dir = DMA_BIDIRECTIONAL;
	else
		q->dma_dir = q->is_output ? DMA_TO_DEVICE : DMA_FROM_DEVICE;

	if (q->name[0] == '\0')
		snprintf(q->name, sizeof(q->name), "%s-%p",
			 q->is_output ? "out" : "cap", q);

	return 0;
}


static int __vb2_init_fileio(struct vb2_queue *q, int read);
static int __vb2_cleanup_fileio(struct vb2_queue *q);
void vb2_core_queue_release(struct vb2_queue *q)
{
	__vb2_cleanup_fileio(q);
	__vb2_queue_cancel(q);
	mutex_lock(&q->mmap_lock);
	__vb2_queue_free(q, q->num_buffers);
	mutex_unlock(&q->mmap_lock);
}


__poll_t vb2_core_poll(struct vb2_queue *q, struct file *file,
		poll_table *wait)
{
	__poll_t req_events = poll_requested_events(wait);
	struct vb2_buffer *vb = NULL;
	unsigned long flags;

	/*
	 * poll_wait() MUST be called on the first invocation on all the
	 * potential queues of interest, even if we are not interested in their
	 * events during this first call. Failure to do so will result in
	 * queue's events to be ignored because the poll_table won't be capable
	 * of adding new wait queues thereafter.
	 */
	poll_wait(file, &q->done_wq, wait);

	if (!q->is_output && !(req_events & (EPOLLIN | EPOLLRDNORM)))
		return 0;
	if (q->is_output && !(req_events & (EPOLLOUT | EPOLLWRNORM)))
		return 0;

	/*
	 * Start file I/O emulator only if streaming API has not been used yet.
	 */
	if (q->num_buffers == 0 && !vb2_fileio_is_active(q)) {
		if (!q->is_output && (q->io_modes & VB2_READ) &&
				(req_events & (EPOLLIN | EPOLLRDNORM))) {
			if (__vb2_init_fileio(q, 1))
				return EPOLLERR;
		}
		if (q->is_output && (q->io_modes & VB2_WRITE) &&
				(req_events & (EPOLLOUT | EPOLLWRNORM))) {
			if (__vb2_init_fileio(q, 0))
				return EPOLLERR;
			/*
			 * Write to OUTPUT queue can be done immediately.
			 */
			return EPOLLOUT | EPOLLWRNORM;
		}
	}

	/*
	 * There is nothing to wait for if the queue isn't streaming, or if the
	 * error flag is set.
	 */
	if (!vb2_is_streaming(q) || q->error)
		return EPOLLERR;

	/*
	 * If this quirk is set and QBUF hasn't been called yet then
	 * return EPOLLERR as well. This only affects capture queues, output
	 * queues will always initialize waiting_for_buffers to false.
	 * This quirk is set by V4L2 for backwards compatibility reasons.
	 */
	if (q->quirk_poll_must_check_waiting_for_buffers &&
	    q->waiting_for_buffers && (req_events & (EPOLLIN | EPOLLRDNORM)))
		return EPOLLERR;

	/*
	 * For output streams you can call write() as long as there are fewer
	 * buffers queued than there are buffers available.
	 */
	if (q->is_output && q->fileio && q->queued_count < q->num_buffers)
		return EPOLLOUT | EPOLLWRNORM;

	if (list_empty(&q->done_list)) {
		/*
		 * If the last buffer was dequeued from a capture queue,
		 * return immediately. DQBUF will return -EPIPE.
		 */
		if (q->last_buffer_dequeued)
			return EPOLLIN | EPOLLRDNORM;
	}

	/*
	 * Take first buffer available for dequeuing.
	 */
	spin_lock_irqsave(&q->done_lock, flags);
	if (!list_empty(&q->done_list))
		vb = list_first_entry(&q->done_list, struct vb2_buffer,
					done_entry);
	spin_unlock_irqrestore(&q->done_lock, flags);

	if (vb && (vb->state == VB2_BUF_STATE_DONE
			|| vb->state == VB2_BUF_STATE_ERROR)) {
		return (q->is_output) ?
				EPOLLOUT | EPOLLWRNORM :
				EPOLLIN | EPOLLRDNORM;
	}
	return 0;
}


/*
 * struct vb2_fileio_buf - buffer context used by file io emulator
 *
 * vb2 provides a compatibility layer and emulator of file io (read and
 * write) calls on top of streaming API. This structure is used for
 * tracking context related to the buffers.
 */
struct vb2_fileio_buf {
	void *vaddr;
	unsigned int size;
	unsigned int pos;
	unsigned int queued:1;
};

/*
 * struct vb2_fileio_data - queue context used by file io emulator
 *
 * @cur_index:	the index of the buffer currently being read from or
 *		written to. If equal to q->num_buffers then a new buffer
 *		must be dequeued.
 * @initial_index: in the read() case all buffers are queued up immediately
 *		in __vb2_init_fileio() and __vb2_perform_fileio() just cycles
 *		buffers. However, in the write() case no buffers are initially
 *		queued, instead whenever a buffer is full it is queued up by
 *		__vb2_perform_fileio(). Only once all available buffers have
 *		been queued up will __vb2_perform_fileio() start to dequeue
 *		buffers. This means that initially __vb2_perform_fileio()
 *		needs to know what buffer index to use when it is queuing up
 *		the buffers for the first time. That initial index is stored
 *		in this field. Once it is equal to q->num_buffers all
 *		available buffers have been queued and __vb2_perform_fileio()
 *		should start the normal dequeue/queue cycle.
 *
 * vb2 provides a compatibility layer and emulator of file io (read and
 * write) calls on top of streaming API. For proper operation it required
 * this structure to save the driver state between each call of the read
 * or write function.
 */
struct vb2_fileio_data {
	unsigned int count;
	unsigned int type;
	unsigned int memory;
	struct vb2_fileio_buf bufs[VB2_MAX_FRAME];
	unsigned int cur_index;
	unsigned int initial_index;
	unsigned int q_count;
	unsigned int dq_count;
	unsigned read_once:1;
	unsigned write_immediately:1;
};

/*
 * __vb2_init_fileio() - initialize file io emulator
 * @q:		videobuf2 queue
 * @read:	mode selector (1 means read, 0 means write)
 */
static int __vb2_init_fileio(struct vb2_queue *q, int read)
{
	struct vb2_fileio_data *fileio;
	int i, ret;
	unsigned int count = 0;

	/*
	 * Sanity check
	 */
	if (WARN_ON((read && !(q->io_modes & VB2_READ)) ||
		    (!read && !(q->io_modes & VB2_WRITE))))
		return -EINVAL;

	/*
	 * Check if device supports mapping buffers to kernel virtual space.
	 */
	if (!q->mem_ops->vaddr)
		return -EBUSY;

	/*
	 * Check if streaming api has not been already activated.
	 */
	if (q->streaming || q->num_buffers > 0)
		return -EBUSY;

	/*
	 * Start with count 1, driver can increase it in queue_setup()
	 */
	count = 1;

	dprintk(q, 3, "setting up file io: mode %s, count %d, read_once %d, write_immediately %d\n",
		(read) ? "read" : "write", count, q->fileio_read_once,
		q->fileio_write_immediately);

	fileio = kzalloc(sizeof(*fileio), GFP_KERNEL);
	if (fileio == NULL)
		return -ENOMEM;

	fileio->read_once = q->fileio_read_once;
	fileio->write_immediately = q->fileio_write_immediately;

	/*
	 * Request buffers and use MMAP type to force driver
	 * to allocate buffers by itself.
	 */
	fileio->count = count;
	fileio->memory = VB2_MEMORY_MMAP;
	fileio->type = q->type;
	q->fileio = fileio;
	ret = vb2_core_reqbufs(q, fileio->memory, &fileio->count);
	if (ret)
		goto err_kfree;

	/*
	 * Check if plane_count is correct
	 * (multiplane buffers are not supported).
	 */
	if (q->bufs[0]->num_planes != 1) {
		ret = -EBUSY;
		goto err_reqbufs;
	}

	/*
	 * Get kernel address of each buffer.
	 */
	for (i = 0; i < q->num_buffers; i++) {
		fileio->bufs[i].vaddr = vb2_plane_vaddr(q->bufs[i], 0);
		if (fileio->bufs[i].vaddr == NULL) {
			ret = -EINVAL;
			goto err_reqbufs;
		}
		fileio->bufs[i].size = vb2_plane_size(q->bufs[i], 0);
	}

	/*
	 * Read mode requires pre queuing of all buffers.
	 */
	if (read) {
		/*
		 * Queue all buffers.
		 */
		for (i = 0; i < q->num_buffers; i++) {
			ret = vb2_core_qbuf(q, i, NULL, NULL);
			if (ret)
				goto err_reqbufs;
			fileio->bufs[i].queued = 1;
		}
		/*
		 * All buffers have been queued, so mark that by setting
		 * initial_index to q->num_buffers
		 */
		fileio->initial_index = q->num_buffers;
		fileio->cur_index = q->num_buffers;
	}

	/*
	 * Start streaming.
	 */
	ret = vb2_core_streamon(q, q->type);
	if (ret)
		goto err_reqbufs;

	return ret;

err_reqbufs:
	fileio->count = 0;
	vb2_core_reqbufs(q, fileio->memory, &fileio->count);

err_kfree:
	q->fileio = NULL;
	kfree(fileio);
	return ret;
}

/*
 * __vb2_cleanup_fileio() - free resourced used by file io emulator
 * @q:		videobuf2 queue
 */
static int __vb2_cleanup_fileio(struct vb2_queue *q)
{
	struct vb2_fileio_data *fileio = q->fileio;

	if (fileio) {
		vb2_core_streamoff(q, q->type);
		q->fileio = NULL;
		fileio->count = 0;
		vb2_core_reqbufs(q, fileio->memory, &fileio->count);
		kfree(fileio);
		dprintk(q, 3, "file io emulator closed\n");
	}
	return 0;
}

/*
 * __vb2_perform_fileio() - perform a single file io (read or write) operation
 * @q:		videobuf2 queue
 * @data:	pointed to target userspace buffer
 * @count:	number of bytes to read or write
 * @ppos:	file handle position tracking pointer
 * @nonblock:	mode selector (1 means blocking calls, 0 means nonblocking)
 * @read:	access mode selector (1 means read, 0 means write)
 */
static size_t __vb2_perform_fileio(struct vb2_queue *q, char __user *data, size_t count,
		loff_t *ppos, int nonblock, int read)
{
	struct vb2_fileio_data *fileio;
	struct vb2_fileio_buf *buf;
	bool is_multiplanar = q->is_multiplanar;
	/*
	 * When using write() to write data to an output video node the vb2 core
	 * should copy timestamps if V4L2_BUF_FLAG_TIMESTAMP_COPY is set. Nobody
	 * else is able to provide this information with the write() operation.
	 */
	bool copy_timestamp = !read && q->copy_timestamp;
	unsigned index;
	int ret;

	dprintk(q, 3, "mode %s, offset %ld, count %zd, %sblocking\n",
		read ? "read" : "write", (long)*ppos, count,
		nonblock ? "non" : "");

	if (!data)
		return -EINVAL;

	if (q->waiting_in_dqbuf) {
		dprintk(q, 3, "another dup()ped fd is %s\n",
			read ? "reading" : "writing");
		return -EBUSY;
	}

	/*
	 * Initialize emulator on first call.
	 */
	if (!vb2_fileio_is_active(q)) {
		ret = __vb2_init_fileio(q, read);
		dprintk(q, 3, "vb2_init_fileio result: %d\n", ret);
		if (ret)
			return ret;
	}
	fileio = q->fileio;

	/*
	 * Check if we need to dequeue the buffer.
	 */
	index = fileio->cur_index;
	if (index >= q->num_buffers) {
		struct vb2_buffer *b;

		/*
		 * Call vb2_dqbuf to get buffer back.
		 */
		ret = vb2_core_dqbuf(q, &index, NULL, nonblock);
		dprintk(q, 5, "vb2_dqbuf result: %d\n", ret);
		if (ret)
			return ret;
		fileio->dq_count += 1;

		fileio->cur_index = index;
		buf = &fileio->bufs[index];
		b = q->bufs[index];

		/*
		 * Get number of bytes filled by the driver
		 */
		buf->pos = 0;
		buf->queued = 0;
		buf->size = read ? vb2_get_plane_payload(q->bufs[index], 0)
				 : vb2_plane_size(q->bufs[index], 0);
		/* Compensate for data_offset on read in the multiplanar case. */
		if (is_multiplanar && read &&
				b->planes[0].data_offset < buf->size) {
			buf->pos = b->planes[0].data_offset;
			buf->size -= buf->pos;
		}
	} else {
		buf = &fileio->bufs[index];
	}

	/*
	 * Limit count on last few bytes of the buffer.
	 */
	if (buf->pos + count > buf->size) {
		count = buf->size - buf->pos;
		dprintk(q, 5, "reducing read count: %zd\n", count);
	}

	/*
	 * Transfer data to userspace.
	 */
	dprintk(q, 3, "copying %zd bytes - buffer %d, offset %u\n",
		count, index, buf->pos);
	if (read)
		ret = copy_to_user(data, buf->vaddr + buf->pos, count);
	else
		ret = copy_from_user(buf->vaddr + buf->pos, data, count);
	if (ret) {
		dprintk(q, 3, "error copying data\n");
		return -EFAULT;
	}

	/*
	 * Update counters.
	 */
	buf->pos += count;
	*ppos += count;

	/*
	 * Queue next buffer if required.
	 */
	if (buf->pos == buf->size || (!read && fileio->write_immediately)) {
		struct vb2_buffer *b = q->bufs[index];

		/*
		 * Check if this is the last buffer to read.
		 */
		if (read && fileio->read_once && fileio->dq_count == 1) {
			dprintk(q, 3, "read limit reached\n");
			return __vb2_cleanup_fileio(q);
		}

		/*
		 * Call vb2_qbuf and give buffer to the driver.
		 */
		b->planes[0].bytesused = buf->pos;

		if (copy_timestamp)
			b->timestamp = ktime_get_ns();
		ret = vb2_core_qbuf(q, index, NULL, NULL);
		dprintk(q, 5, "vb2_dbuf result: %d\n", ret);
		if (ret)
			return ret;

		/*
		 * Buffer has been queued, update the status
		 */
		buf->pos = 0;
		buf->queued = 1;
		buf->size = vb2_plane_size(q->bufs[index], 0);
		fileio->q_count += 1;
		/*
		 * If we are queuing up buffers for the first time, then
		 * increase initial_index by one.
		 */
		if (fileio->initial_index < q->num_buffers)
			fileio->initial_index++;
		/*
		 * The next buffer to use is either a buffer that's going to be
		 * queued for the first time (initial_index < q->num_buffers)
		 * or it is equal to q->num_buffers, meaning that the next
		 * time we need to dequeue a buffer since we've now queued up
		 * all the 'first time' buffers.
		 */
		fileio->cur_index = fileio->initial_index;
	}

	/*
	 * Return proper number of bytes processed.
	 */
	if (ret == 0)
		ret = count;
	return ret;
}

size_t vb2_read(struct vb2_queue *q, char __user *data, size_t count,
		loff_t *ppos, int nonblocking)
{
	return __vb2_perform_fileio(q, data, count, ppos, nonblocking, 1);
}


size_t vb2_write(struct vb2_queue *q, const char __user *data, size_t count,
		loff_t *ppos, int nonblocking)
{
	return __vb2_perform_fileio(q, (char __user *) data, count,
							ppos, nonblocking, 0);
}


struct vb2_threadio_data {
	struct task_struct *thread;
	vb2_thread_fnc fnc;
	void *priv;
	bool stop;
};

static int vb2_thread(void *data)
{
	struct vb2_queue *q = data;
	struct vb2_threadio_data *threadio = q->threadio;
	bool copy_timestamp = false;
	unsigned prequeue = 0;
	unsigned index = 0;
	int ret = 0;

	if (q->is_output) {
		prequeue = q->num_buffers;
		copy_timestamp = q->copy_timestamp;
	}

	set_freezable();

	for (;;) {
		struct vb2_buffer *vb;

		/*
		 * Call vb2_dqbuf to get buffer back.
		 */
		if (prequeue) {
			vb = q->bufs[index++];
			prequeue--;
		} else {
			call_void_qop(q, wait_finish, q);
			if (!threadio->stop)
				ret = vb2_core_dqbuf(q, &index, NULL, 0);
			call_void_qop(q, wait_prepare, q);
			dprintk(q, 5, "file io: vb2_dqbuf result: %d\n", ret);
			if (!ret)
				vb = q->bufs[index];
		}
		if (ret || threadio->stop)
			break;
		try_to_freeze();

		if (vb->state != VB2_BUF_STATE_ERROR)
			if (threadio->fnc(vb, threadio->priv))
				break;
		call_void_qop(q, wait_finish, q);
		if (copy_timestamp)
			vb->timestamp = ktime_get_ns();
		if (!threadio->stop)
			ret = vb2_core_qbuf(q, vb->index, NULL, NULL);
		call_void_qop(q, wait_prepare, q);
		if (ret || threadio->stop)
			break;
	}

	/* Hmm, linux becomes *very* unhappy without this ... */
	while (!kthread_should_stop()) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
	}
	return 0;
}

/*
 * This function should not be used for anything else but the videobuf2-dvb
 * support. If you think you have another good use-case for this, then please
 * contact the linux-media mailinglist first.
 */
int vb2_thread_start(struct vb2_queue *q, vb2_thread_fnc fnc, void *priv,
		     const char *thread_name)
{
	struct vb2_threadio_data *threadio;
	int ret = 0;

	if (q->threadio)
		return -EBUSY;
	if (vb2_is_busy(q))
		return -EBUSY;
	if (WARN_ON(q->fileio))
		return -EBUSY;

	threadio = kzalloc(sizeof(*threadio), GFP_KERNEL);
	if (threadio == NULL)
		return -ENOMEM;
	threadio->fnc = fnc;
	threadio->priv = priv;

	ret = __vb2_init_fileio(q, !q->is_output);
	dprintk(q, 3, "file io: vb2_init_fileio result: %d\n", ret);
	if (ret)
		goto nomem;
	q->threadio = threadio;
	threadio->thread = kthread_run(vb2_thread, q, "vb2-%s", thread_name);
	if (IS_ERR(threadio->thread)) {
		ret = PTR_ERR(threadio->thread);
		threadio->thread = NULL;
		goto nothread;
	}
	return 0;

nothread:
	__vb2_cleanup_fileio(q);
nomem:
	kfree(threadio);
	return ret;
}


int vb2_thread_stop(struct vb2_queue *q)
{
	struct vb2_threadio_data *threadio = q->threadio;
	int err;

	if (threadio == NULL)
		return 0;
	threadio->stop = true;
	/* Wake up all pending sleeps in the thread */
	vb2_queue_error(q);
	err = kthread_stop(threadio->thread);
	__vb2_cleanup_fileio(q);
	threadio->thread = NULL;
	kfree(threadio);
	q->threadio = NULL;
	return err;
}


//MODULE_DESCRIPTION("Media buffer core framework");
//MODULE_AUTHOR("Pawel Osciak <pawel@osciak.com>, Marek Szyprowski");
//MODULE_LICENSE("GPL");






//#undef EXPORT_SYMBOL_GPL(X)
//#undef MODULE_DESCRIPTION(X)
//#undef MODULE_AUTHOR(X);
//#undef MODULE_LICENSE(X);






///////////////////////////////////////////////////////////////////////////////////////////////////////
#endif


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
	ret = video_register_device(&sample_v4l2_dev.sample_video_dev , VFL_TYPE_VIDEO , 0);
	printk(KERN_ALERT "sample_v4l2_dev.sample_video_dev.vfl_dir = %d\r\n" , sample_v4l2_dev.sample_video_dev.vfl_dir);
	printk(KERN_ALERT "sample_v4l2_dev.sample_video_dev.dev.kobj.name = %s\r\n" , sample_v4l2_dev.sample_video_dev.dev.kobj.name);
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