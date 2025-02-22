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
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x20\x00\x00\x00\x2f\x72\x7f\x21"
	"unregister_framebuffer\0\0"
	"\x1c\x00\x00\x00\x23\x34\xa8\xab"
	"framebuffer_release\0"
	"\x1c\x00\x00\x00\x3b\x07\x8b\x97"
	"framebuffer_alloc\0\0\0"
	"\x18\x00\x00\x00\x1b\xda\x3d\xdb"
	"dma_alloc_attrs\0"
	"\x20\x00\x00\x00\xa9\xd9\x12\xf0"
	"register_framebuffer\0\0\0\0"
	"\x14\x00\x00\x00\x10\xf5\xd0\x5b"
	"_dev_err\0\0\0\0"
	"\x24\x00\x00\x00\xda\x90\x30\x27"
	"platform_driver_unregister\0\0"
	"\x24\x00\x00\x00\x70\x99\x14\xb8"
	"platform_device_unregister\0\0"
	"\x1c\x00\x00\x00\xb0\x59\xa6\xe2"
	"platform_device_put\0"
	"\x14\x00\x00\x00\x94\xc7\x86\xdb"
	"fb_io_read\0\0"
	"\x14\x00\x00\x00\x18\xcb\x33\x74"
	"fb_io_write\0"
	"\x18\x00\x00\x00\xee\xc9\x3b\x75"
	"cfb_fillrect\0\0\0\0"
	"\x18\x00\x00\x00\x1a\x3e\xd2\xb8"
	"cfb_copyarea\0\0\0\0"
	"\x18\x00\x00\x00\x9d\x14\xb4\x3e"
	"cfb_imageblit\0\0\0"
	"\x14\x00\x00\x00\xac\x3c\x41\x02"
	"fb_io_mmap\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x10\x00\x00\x00\x7e\x3a\x2c\x12"
	"_printk\0"
	"\x20\x00\x00\x00\xcf\x45\xc0\x06"
	"platform_device_alloc\0\0\0"
	"\x24\x00\x00\x00\x36\x14\x41\xa0"
	"platform_device_register\0\0\0\0"
	"\x18\x00\x00\x00\xd7\xd3\x75\x6d"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "54BE406FC81C9B931725DC6");
