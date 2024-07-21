#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
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

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x3730b1bc, "module_layout" },
	{ 0x69522567, "kmalloc_caches" },
	{ 0xd18686b8, "devm_gpiochip_add_data" },
	{ 0x1c1454f9, "gpiod_remove_lookup_table" },
	{ 0x1d4e2af4, "gpiod_add_lookup_table" },
	{ 0x15ba50a6, "jiffies" },
	{ 0x584e594c, "input_event" },
	{ 0x8384d66f, "device_del" },
	{ 0x7c32d0f0, "printk" },
	{ 0xd9ef27a6, "input_set_capability" },
	{ 0xc38c83b8, "mod_timer" },
	{ 0xf703d2fe, "device_add" },
	{ 0xdf737ca0, "bgpio_init" },
	{ 0x9a653b21, "input_free_device" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xa1bf772d, "kmem_cache_alloc_trace" },
	{ 0x9dbdfd9, "gpiod_get_value" },
	{ 0x37a0cba, "kfree" },
	{ 0xf98b0923, "device_initialize" },
	{ 0x5204775, "dev_set_name" },
	{ 0x632581, "gpiod_set_value" },
	{ 0x32e331e9, "devm_gpiochip_remove" },
	{ 0x8760838d, "gpiod_put" },
	{ 0x6132c931, "gpiod_get" },
	{ 0xae905b72, "input_allocate_device" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=gpio-generic";

