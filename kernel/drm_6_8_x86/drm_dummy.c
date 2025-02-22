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
#include <drm/drm_fbdev_generic.h>
#include <drm/drm_gem_dma_helper.h>
#include <linux/platform_device.h>


static struct drm_device* drm = NULL;
static struct drm_plane primary;
static struct drm_crtc crtc;
static struct drm_encoder encoder;
static struct drm_connector connector;
static struct hrtimer vblank_hrtimer;
//static struct device dev;
static struct platform_device* pdev = NULL;


#if 0
static enum hrtimer_restart vkms_vblank_simulate(struct hrtimer *timer)
{
 drm_crtc_handle_vblank(&crtc);

 hrtimer_forward_now(&vblank_hrtimer, 16666667);

 return HRTIMER_RESTART;
}
#endif






static void vkms_crtc_atomic_enable(struct drm_crtc *crtc,
				    struct drm_atomic_state *state)
{
	printk(KERN_ALERT "vkms_crtc_atomic_enable\r\n");
}

static void vkms_crtc_atomic_disable(struct drm_crtc *crtc,
				    struct drm_atomic_state *state)
{
	printk(KERN_ALERT "vkms_crtc_atomic_disable\r\n");
}

static void vkms_crtc_atomic_flush(struct drm_crtc *crtc,
				    struct drm_atomic_state *state)
{
 unsigned long flags;

 if (crtc->state->event) {
  spin_lock_irqsave(&crtc->dev->event_lock, flags);
  drm_crtc_send_vblank_event(crtc, crtc->state->event);
  spin_unlock_irqrestore(&crtc->dev->event_lock, flags);

  crtc->state->event = NULL;
 }
}

static const struct drm_crtc_helper_funcs vkms_crtc_helper_funcs = {
 .atomic_enable = vkms_crtc_atomic_enable,
 .atomic_disable = vkms_crtc_atomic_disable,
 .atomic_flush = vkms_crtc_atomic_flush,
};

static const struct drm_crtc_funcs vkms_crtc_funcs = {
 .set_config             = drm_atomic_helper_set_config,
 .page_flip              = drm_atomic_helper_page_flip,
 .destroy                = drm_crtc_cleanup,
 .reset                  = drm_atomic_helper_crtc_reset,
 .atomic_duplicate_state = drm_atomic_helper_crtc_duplicate_state,
 .atomic_destroy_state   = drm_atomic_helper_crtc_destroy_state,
};

static void vkms_plane_atomic_update(struct drm_plane *plane,
				     struct drm_atomic_state *state)
{
	printk(KERN_ALERT "vkms_plane_atomic_update!\r\n");
}

static const struct drm_plane_helper_funcs vkms_plane_helper_funcs = {
 .atomic_update  = vkms_plane_atomic_update,
};

static const struct drm_plane_funcs vkms_plane_funcs = {
 .update_plane  = drm_atomic_helper_update_plane,
 .disable_plane  = drm_atomic_helper_disable_plane,
 .destroy   = drm_plane_cleanup,
 .reset    = drm_atomic_helper_plane_reset,
 .atomic_duplicate_state = drm_atomic_helper_plane_duplicate_state,
 .atomic_destroy_state = drm_atomic_helper_plane_destroy_state,
};

static int vkms_conn_get_modes(struct drm_connector *connector)
{
	int count;

	count = drm_add_modes_noedid(connector, 8192, 8192);
	drm_set_preferred_mode(connector, 1024, 768);

	return count;
}

static const struct drm_connector_helper_funcs vkms_conn_helper_funcs = {
 .get_modes = vkms_conn_get_modes,
};

static const struct drm_connector_funcs vkms_connector_funcs = {
 .fill_modes = drm_helper_probe_single_connector_modes,
 .destroy = drm_connector_cleanup,
 .reset = drm_atomic_helper_connector_reset,
 .atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
 .atomic_destroy_state = drm_atomic_helper_connector_destroy_state,
};

static const struct drm_encoder_funcs vkms_encoder_funcs = {
 .destroy = drm_encoder_cleanup,
};

static const struct drm_mode_config_funcs vkms_mode_funcs = {
 .fb_create = drm_gem_fb_create,
 .atomic_check = drm_atomic_helper_check,
 .atomic_commit = drm_atomic_helper_commit,
};

static const u32 vkms_formats[] = {
 DRM_FORMAT_XRGB8888,
};

static void vkms_modeset_init(void)
{
 drm_mode_config_init(drm);
 drm->mode_config.max_width = 8192;
 drm->mode_config.max_height = 8192;
 drm->mode_config.funcs = &vkms_mode_funcs;

 drm_universal_plane_init(drm, &primary, 0, &vkms_plane_funcs,
     vkms_formats, ARRAY_SIZE(vkms_formats),
     NULL, DRM_PLANE_TYPE_PRIMARY, NULL);
 drm_plane_helper_add(&primary, &vkms_plane_helper_funcs);

 drm_crtc_init_with_planes(drm, &crtc, &primary, NULL, &vkms_crtc_funcs, NULL);
 drm_crtc_helper_add(&crtc, &vkms_crtc_helper_funcs);

 drm_encoder_init(drm, &encoder, &vkms_encoder_funcs, DRM_MODE_ENCODER_VIRTUAL, NULL);

 drm_connector_init(drm, &connector, &vkms_connector_funcs, DRM_MODE_CONNECTOR_VIRTUAL);
 drm_connector_helper_add(&connector, &vkms_conn_helper_funcs);
 drm_connector_attach_encoder(&connector, &encoder);

 drm_mode_config_reset(drm);
}


int clone_drm_open(struct inode *inode, struct file *filp)
{
	printk(KERN_EMERG "clone_drm_open\r\n");
	return 0;
}


static const struct file_operations vkms_fops = {
 .owner = THIS_MODULE,
 .open = drm_open,
 .release = drm_release,
 .unlocked_ioctl = drm_ioctl,
 .poll = drm_poll,
 .read = drm_read,
 .mmap = drm_gem_mmap,
};


static struct drm_driver vkms_driver = {
	//.driver_features = DRIVER_MODESET | DRIVER_GEM | DRIVER_ATOMIC,
	.driver_features = DRIVER_MODESET | DRIVER_GEM ,
  .fops   = &vkms_fops,
	DRM_GEM_DMA_DRIVER_OPS,
	.name   = "dummy_drm",
	.desc   = "Virtual Kernel Mode Setting",
	.date   = "20180514", 
 //.major   = 1,
 //.minor   = 0,
};

unsigned int* base_buffer;
static struct timer_list my_timer;
static struct drm_gem_dma_object *dma_obj;
// 定时器处理函数
void my_timer_handler(struct timer_list *t)
{
	for(int i = 0 ; i < 10 ; i++)
	{
		//printk(KERN_ALERT "((unsigned int*)(dma_obj->vaddr))[%d] = %d\r\n" , i , ((unsigned int*)(dma_obj->vaddr))[i]);
		printk(KERN_ALERT "(base_buffer)[%d] = %d\r\n" , i , base_buffer[i]);
	}
	
	mod_timer(&my_timer, jiffies + msecs_to_jiffies(3000));
}



static int dummy_drm_probe(struct platform_device *pdev)
{
	printk(KERN_ALERT "dummy_drm_probe\r\n");
	int ret = 0;
	// dev_set_name(&dev , "test");
	// device_register(&dev);
	drm = drm_dev_alloc(&vkms_driver, &pdev->dev);

	drm_mode_config_init(drm);
	drm->mode_config.min_width = 32;
	drm->mode_config.min_height = 32;
	drm->mode_config.max_width = 8192;
	drm->mode_config.max_height = 8192;
	drm->mode_config.funcs = &vkms_mode_funcs;
	//drm->mode_config.helper_private = &vkms_mode_config_helpers;

	ret = drm_universal_plane_init(drm, &primary, 0, &vkms_plane_funcs,
			vkms_formats, ARRAY_SIZE(vkms_formats),
			NULL, DRM_PLANE_TYPE_PRIMARY, NULL);
	printk(KERN_EMERG "drm_universal_plane_init ret = %d\r\n" , ret);

	drm_plane_helper_add(&primary, &vkms_plane_helper_funcs);
	ret = drm_crtc_init_with_planes(drm, &crtc, &primary, NULL, &vkms_crtc_funcs, NULL);
	printk(KERN_EMERG "drm_crtc_init_with_planes ret = %d\r\n" , ret);
	
	drm_crtc_helper_add(&crtc, &vkms_crtc_helper_funcs);
 	
	drm_encoder_init(drm, &encoder, &vkms_encoder_funcs, DRM_MODE_ENCODER_VIRTUAL, NULL);
	encoder.possible_crtcs = 1;
	drm_connector_init(drm, &connector, &vkms_connector_funcs, DRM_MODE_CONNECTOR_VIRTUAL);
	drm_connector_helper_add(&connector, &vkms_conn_helper_funcs);
	
	drm_connector_attach_encoder(&connector, &encoder);
	
	drm_mode_config_reset(drm);
	
	drm_dev_register(drm, 0);

	drm_fbdev_generic_setup(drm , 0);

	dma_obj = container_of(drm->fb_helper->buffer->gem , struct drm_gem_dma_object , base);
	//struct drm_gem_object *gem_obj;
	
	base_buffer = (unsigned int*)(drm->fb_helper->info->screen_buffer);
	printk(KERN_ALERT "base_buffer = 0x%llx\r\n" , base_buffer);
	printk(KERN_ALERT "dma_obj->vaddr = 0x%llx\r\n" , dma_obj->vaddr);


	for(int i = 0 ; i < 10 ; i++)
		((unsigned int*)(dma_obj->vaddr))[i] = i*10;

	//timer_setup(&my_timer, my_timer_handler, 0);
	//mod_timer(&my_timer, jiffies + msecs_to_jiffies(1000));



	return 0;
}



static int dummy_drm_remove(struct platform_device *pdev)  
{  
	printk(KERN_ALERT "dummy_drm_remove\r\n");
	drm_dev_unregister(drm);
	drm_dev_put(drm);
	//device_unregister(&dev);
	return 0;
}

static struct platform_driver pdrv = {  
    .probe = dummy_drm_probe,  
    .remove = dummy_drm_remove,  
    .driver = {  
        .name = "dummy_drm",  
        //.of_match_table = of_match_ptr(my_simple_fb_of_match),  
    },  
};



static int __init dummy_drm_init(void)
{
	printk(KERN_ALERT "vkms_init\r\n");
	pdev = platform_device_alloc("dummy_drm" , -1);
	platform_device_register(pdev);
	platform_driver_register(&pdrv);
	return 0;
}

static void __exit dummy_drm_exit(void) 
{
	printk(KERN_ALERT "dummy_drm_exit\r\n");
	platform_driver_unregister(&pdrv);
	platform_device_unregister(pdev);
	platform_device_put(pdev);
}


module_init(dummy_drm_init);
module_exit(dummy_drm_exit);

MODULE_LICENSE("GPL");
