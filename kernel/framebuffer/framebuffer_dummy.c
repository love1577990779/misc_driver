#include <drm/drm_atomic_helper.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_drv.h>
#include <drm/drm_vblank.h>
// #include <drm/drm_fb_cma_helper.h>
// #include <drm/drm_gem_cma_helper.h>
#include <linux/hrtimer.h>
#include <drm/drm_edid.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_ioctl.h>
#include <drm/drm_connector.h>
#include <drm/drm_gem.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_gem_shmem_helper.h>
#include <drm/drm_fbdev_generic.h>
#include <drm/drm_gem_shmem_helper.h>
#include <linux/fb.h>
#include <linux/platform_device.h>


#define SCREEN_W        (800)
#define SCREEN_H        (600)
#define BITS_PER_PIXEL	(32)

static struct platform_device* pdev;
static struct fb_info* info;



static int dummy_fb_mmap(struct fb_info *info, struct vm_area_struct *vma) 
{
	printk(KERN_ALERT "dummy_fb_mmap\r\n");
	return 0;
}




static struct fb_ops dummy_fb_ops = {
        .owner          = THIS_MODULE,
        //.fb_setcolreg = s3c_setcolreg,
        // .fb_fillrect    = cfb_fillrect,
        // .fb_copyarea    = cfb_copyarea,
        // .fb_imageblit   = cfb_imageblit,
				//.fb_mmap = dummy_fb_mmap,
				FB_DEFAULT_IOMEM_OPS,
};


static int dummy_fb_probe(struct platform_device *pdev)  
{  
	printk(KERN_ALERT "dummy_fb_probe\r\n");
	int ret;  

	info = framebuffer_alloc(0, &pdev->dev);  
	if (!info) {  
			ret = -ENOMEM;  
			dev_err(&pdev->dev, "Cannot allocate framebuffer\n");  
			return ret;  
	}  

	info->fbops = &dummy_fb_ops;  
	info->var.xres = SCREEN_W;  
	info->var.yres = SCREEN_H;  
	info->var.bits_per_pixel = BITS_PER_PIXEL;

	info->var.red.offset   = 0x10;
  info->var.red.length   = 0x8;
  info->var.green.offset = 0x8;
  info->var.green.length = 0x8;
  info->var.blue.offset  = 0;
  info->var.blue.length  = 0x8;
	info->var.transp.offset=0x0;
	info->var.transp.length=0x0;
	//info->var.transp

	info->fix.line_length = info->var.yres * info->var.bits_per_pixel / 8;
	info->fix.smem_len = SCREEN_W * SCREEN_H * BITS_PER_PIXEL / 8;
	info->screen_base = dma_alloc_wc(&pdev->dev , info->fix.smem_len , (dma_addr_t*)&info->fix.smem_start , GFP_KERNEL);
	printk(KERN_ALERT "info->screen_base = 0x%x\r\n" , info->screen_base);
	printk(KERN_ALERT "info->fix.smem_start = 0x%x\r\n" , info->fix.smem_start);

	info->screen_size = info->fix.smem_len;
	info->fix.type   = FB_TYPE_PACKED_PIXELS;
  info->fix.visual = FB_VISUAL_TRUECOLOR;
	info->fix.ypanstep = 1;

	// 设置其他framebuffer参数...  
	ret = register_framebuffer(info);  
	if (ret < 0) {  
			dev_err(&pdev->dev, "Cannot register framebuffer\r\n");  
			return -1;  
	}  


	int mmio_pgoff = PAGE_ALIGN((info->fix.smem_start & ~PAGE_MASK) + info->fix.smem_len) >> PAGE_SHIFT;
	printk(KERN_ALERT "mmio_pgoff = %d\r\n" , mmio_pgoff);

	printk(KERN_ALERT "info->fbdefio = %d\r\n" , info->fbdefio);
	printk(KERN_ALERT "info->var.accel_flags = %d\r\n" , info->var.accel_flags);
	//dev_info(&pdev->dev, "Simple framebuffer driver loaded\n");  

	return 0;
}  


static int dummy_fb_remove(struct platform_device *pdev)  
{  
	for(int i = 0 ; i < 10 ; i++)
	{
		printk(KERN_ALERT "screen_base[i] = 0x%x\r\n" , info->screen_base[i]);
	}


	unregister_framebuffer(info);  
	framebuffer_release(info);  
	return 0;  
}

static struct platform_driver dummy_fb_driver = {  
    .probe = dummy_fb_probe,  
    .remove = dummy_fb_remove,  
    .driver = {  
        .name = "dummy_fb",  
        //.of_match_table = of_match_ptr(my_simple_fb_of_match),  
    },  
};


static int __init framebuffer_init(void)
{
	printk(KERN_ALERT "framebuffer_init\r\n");
	pdev = platform_device_alloc("dummy_fb" , -1);
	platform_device_register(pdev);
	platform_driver_register(&dummy_fb_driver);
	
	return 0;
}

static void __exit framebuffer_exit(void) 
{
	printk(KERN_ALERT "framebuffer_exit\r\n");
	platform_driver_unregister(&dummy_fb_driver);
	platform_device_unregister(pdev);
	platform_device_put(pdev);
}


module_init(framebuffer_init);
module_exit(framebuffer_exit);
MODULE_LICENSE("GPL");
