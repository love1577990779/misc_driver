#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const char ____versions[]
__used __section("__versions") =
	"\x24\x00\x00\x00\x5d\x84\xb4\x01"
	"__platform_driver_register\0\0"
	"\x1c\x00\x00\x00\xf4\x88\x8f\x66"
	"drm_dev_unregister\0\0"
	"\x14\x00\x00\x00\xf3\xee\x75\x1c"
	"drm_dev_put\0"
	"\x20\x00\x00\x00\x0b\x7c\x07\xff"
	"drm_add_modes_noedid\0\0\0\0"
	"\x20\x00\x00\x00\x56\x82\xf0\x7e"
	"drm_set_preferred_mode\0\0"
	"\x18\x00\x00\x00\xab\x1e\xb3\x2d"
	"drm_dev_alloc\0\0\0"
	"\x20\x00\x00\x00\xe0\x40\x64\x67"
	"drmm_mode_config_init\0\0\0"
	"\x24\x00\x00\x00\xfe\xb5\x59\x4a"
	"drm_universal_plane_init\0\0\0\0"
	"\x24\x00\x00\x00\xe8\x54\x76\x45"
	"drm_crtc_init_with_planes\0\0\0"
	"\x1c\x00\x00\x00\x37\xe1\xca\x22"
	"drm_encoder_init\0\0\0\0"
	"\x1c\x00\x00\x00\x03\x50\xfb\xd7"
	"drm_connector_init\0\0"
	"\x28\x00\x00\x00\x83\x2c\x7f\xb3"
	"drm_connector_attach_encoder\0\0\0\0"
	"\x20\x00\x00\x00\x6e\x57\x90\x9c"
	"drm_mode_config_reset\0\0\0"
	"\x1c\x00\x00\x00\x35\x6c\x91\x8a"
	"drm_dev_register\0\0\0\0"
	"\x20\x00\x00\x00\x42\x60\x8e\x83"
	"drm_fbdev_generic_setup\0"
	"\x18\x00\x00\x00\x39\x63\xf4\xc6"
	"init_timer_key\0\0"
	"\x24\x00\x00\x00\xda\x90\x30\x27"
	"platform_driver_unregister\0\0"
	"\x24\x00\x00\x00\x70\x99\x14\xb8"
	"platform_device_unregister\0\0"
	"\x1c\x00\x00\x00\xb0\x59\xa6\xe2"
	"platform_device_put\0"
	"\x20\x00\x00\x00\x0b\x05\xdb\x34"
	"_raw_spin_lock_irqsave\0\0"
	"\x24\x00\x00\x00\x86\xc6\x11\xbe"
	"drm_crtc_send_vblank_event\0\0"
	"\x24\x00\x00\x00\x70\xce\x5c\xd3"
	"_raw_spin_unlock_irqrestore\0"
	"\x2c\x00\x00\x00\x9a\x60\x0e\x12"
	"drm_gem_dma_prime_import_sg_table\0\0\0"
	"\x20\x00\x00\x00\x45\x0e\x5b\x3f"
	"drm_gem_dma_dumb_create\0"
	"\x14\x00\x00\x00\x60\x22\xda\x71"
	"drm_read\0\0\0\0"
	"\x14\x00\x00\x00\x90\x1a\x47\x15"
	"drm_poll\0\0\0\0"
	"\x14\x00\x00\x00\x24\x5f\x4b\x0b"
	"drm_ioctl\0\0\0"
	"\x18\x00\x00\x00\x49\x5a\x40\x91"
	"drm_gem_mmap\0\0\0\0"
	"\x14\x00\x00\x00\x09\x8e\x35\xf1"
	"drm_open\0\0\0\0"
	"\x14\x00\x00\x00\x66\xbb\xf2\x92"
	"drm_release\0"
	"\x1c\x00\x00\x00\xe4\x74\x30\x69"
	"drm_gem_fb_create\0\0\0"
	"\x20\x00\x00\x00\x62\x46\x9a\x46"
	"drm_atomic_helper_check\0"
	"\x24\x00\x00\x00\x6c\xad\x29\x71"
	"drm_atomic_helper_commit\0\0\0\0"
	"\x1c\x00\x00\x00\x56\xbe\xd9\xd1"
	"drm_encoder_cleanup\0"
	"\x2c\x00\x00\x00\xc8\xdb\xaf\x0c"
	"drm_atomic_helper_connector_reset\0\0\0"
	"\x30\x00\x00\x00\x03\xa0\x03\xae"
	"drm_helper_probe_single_connector_modes\0"
	"\x20\x00\x00\x00\xc8\x4d\x13\x5d"
	"drm_connector_cleanup\0\0\0"
	"\x34\x00\x00\x00\x16\x7b\xee\x7f"
	"drm_atomic_helper_connector_duplicate_state\0"
	"\x34\x00\x00\x00\xdd\xbe\xb9\x1b"
	"drm_atomic_helper_connector_destroy_state\0\0\0"
	"\x28\x00\x00\x00\xe8\xa2\x95\xfa"
	"drm_atomic_helper_update_plane\0\0"
	"\x28\x00\x00\x00\xdf\x62\x53\xcb"
	"drm_atomic_helper_disable_plane\0"
	"\x1c\x00\x00\x00\xe7\x3a\x18\xc0"
	"drm_plane_cleanup\0\0\0"
	"\x28\x00\x00\x00\x6b\xe9\xb2\xea"
	"drm_atomic_helper_plane_reset\0\0\0"
	"\x30\x00\x00\x00\x79\x69\x92\x3e"
	"drm_atomic_helper_plane_duplicate_state\0"
	"\x30\x00\x00\x00\xb7\x57\x22\x5c"
	"drm_atomic_helper_plane_destroy_state\0\0\0"
	"\x28\x00\x00\x00\xf8\xa1\x47\x09"
	"drm_atomic_helper_crtc_reset\0\0\0\0"
	"\x1c\x00\x00\x00\x2b\x38\xdb\x4e"
	"drm_crtc_cleanup\0\0\0\0"
	"\x28\x00\x00\x00\x90\x19\x94\x9f"
	"drm_atomic_helper_set_config\0\0\0\0"
	"\x24\x00\x00\x00\xd2\x2f\xe6\xde"
	"drm_atomic_helper_page_flip\0"
	"\x30\x00\x00\x00\x0e\x35\xbe\x85"
	"drm_atomic_helper_crtc_duplicate_state\0\0"
	"\x30\x00\x00\x00\xc2\xdf\x9d\xa7"
	"drm_atomic_helper_crtc_destroy_state\0\0\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x10\x00\x00\x00\x7e\x3a\x2c\x12"
	"_printk\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x10\x00\x00\x00\xa6\x50\xba\x15"
	"jiffies\0"
	"\x14\x00\x00\x00\xb8\x83\x8c\xc3"
	"mod_timer\0\0\0"
	"\x20\x00\x00\x00\xcf\x45\xc0\x06"
	"platform_device_alloc\0\0\0"
	"\x24\x00\x00\x00\x36\x14\x41\xa0"
	"platform_device_register\0\0\0\0"
	"\x18\x00\x00\x00\xd7\xd3\x75\x6d"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "drm_dma_helper");


MODULE_INFO(srcversion, "7AF1EDCAB2736435034B983");
