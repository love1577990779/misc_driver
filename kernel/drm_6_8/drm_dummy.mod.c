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
	"\x18\x00\x00\x00\x52\xe7\x39\xcd"
	"device_register\0"
	"\x18\x00\x00\x00\x7e\x86\x65\x1f"
	"drm_dev_alloc\0\0\0"
	"\x20\x00\x00\x00\xe2\xe5\xb9\x32"
	"drmm_mode_config_init\0\0\0"
	"\x24\x00\x00\x00\x1b\x09\xed\x57"
	"drm_universal_plane_init\0\0\0\0"
	"\x1c\x00\x00\x00\xdc\xa5\xe9\xf6"
	"drm_dev_register\0\0\0\0"
	"\x10\x00\x00\x00\x7e\x3a\x2c\x12"
	"_printk\0"
	"\x1c\x00\x00\x00\x5c\x6a\x0a\xeb"
	"drm_dev_unregister\0\0"
	"\x1c\x00\x00\x00\xad\x91\x83\x45"
	"device_unregister\0\0\0"
	"\x1c\x00\x00\x00\xc7\x24\x97\xc5"
	"drm_gem_fb_create\0\0\0"
	"\x20\x00\x00\x00\xe1\x52\x1d\x8e"
	"drm_atomic_helper_check\0"
	"\x24\x00\x00\x00\xe5\xb7\xaf\xe0"
	"drm_atomic_helper_commit\0\0\0\0"
	"\x28\x00\x00\x00\x19\x7d\x0d\x76"
	"drm_atomic_helper_update_plane\0\0"
	"\x28\x00\x00\x00\x85\x63\x85\x7f"
	"drm_atomic_helper_disable_plane\0"
	"\x1c\x00\x00\x00\xd2\x97\x4e\x95"
	"drm_plane_cleanup\0\0\0"
	"\x28\x00\x00\x00\x51\x42\xb6\x95"
	"drm_atomic_helper_plane_reset\0\0\0"
	"\x30\x00\x00\x00\x54\x15\x0d\x3b"
	"drm_atomic_helper_plane_duplicate_state\0"
	"\x30\x00\x00\x00\x72\xc8\x82\x64"
	"drm_atomic_helper_plane_destroy_state\0\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x18\x00\x00\x00\xb0\x1f\xe4\xa8"
	"dev_set_name\0\0\0\0"
	"\x18\x00\x00\x00\xe8\xd8\x3d\xf5"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "E74EB13EBAF9E55705433DD");
