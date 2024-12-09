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

static struct drm_device* drm = NULL;
static struct drm_plane primary;
static struct drm_crtc crtc;
static struct drm_encoder encoder;
static struct drm_connector connector;
static struct hrtimer vblank_hrtimer;
static struct device dev;

static enum hrtimer_restart vkms_vblank_simulate(struct hrtimer *timer)
{
 drm_crtc_handle_vblank(&crtc);

 hrtimer_forward_now(&vblank_hrtimer, 16666667);

 return HRTIMER_RESTART;
}

static void vkms_crtc_atomic_enable(struct drm_crtc *crtc,
				    struct drm_atomic_state *state)
{
	printk(KERN_ALERT "vkms_crtc_atomic_enable\r\n");
}

static void vkms_crtc_atomic_disable(struct drm_crtc *crtc,
				    struct drm_atomic_state *state)
{
 hrtimer_cancel(&vblank_hrtimer);
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

	.dumb_create = drm_gem_shmem_dumb_create,
	.prime_handle_to_fd	= drm_gem_prime_handle_to_fd, 
	.prime_fd_to_handle	= drm_gem_prime_fd_to_handle, 
	.gem_prime_import_sg_table = drm_gem_shmem_prime_import_sg_table, 
	.gem_prime_mmap		= drm_gem_prime_mmap, 
 
	.name   = "dummy_drm",
	.desc   = "Virtual Kernel Mode Setting",
	.date   = "20180514", 
 //.major   = 1,
 //.minor   = 0,
};

static int __init vkms_init(void)
{
	printk(KERN_ALERT "dummy_drm_init\r\n");
	int ret = 0;
	dev_set_name(&dev , "test");
	device_register(&dev);
	drm = drm_dev_alloc(&vkms_driver, &dev);

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
	
	drm_connector_init(drm, &connector, &vkms_connector_funcs, DRM_MODE_CONNECTOR_VIRTUAL);
	drm_connector_helper_add(&connector, &vkms_conn_helper_funcs);
	//drm_connector_attach_encoder(&connector, &encoder);
	
	drm_mode_config_reset(drm);
	
	drm_dev_register(drm, 0);

	drm_fbdev_generic_setup(drm , 0);

 return 0;
}

static void __exit dummy_drm_exit(void) 
{
	printk(KERN_ALERT "dummy_drm_exit\r\n");
	drm_dev_unregister(drm);
	drm_dev_put(drm);
	device_unregister(&dev);
}


module_init(vkms_init);
module_exit(dummy_drm_exit);

MODULE_LICENSE("GPL");