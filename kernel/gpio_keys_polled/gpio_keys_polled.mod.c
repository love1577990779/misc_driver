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
	{ 0x92ae795a, "platform_device_unregister" },
	{ 0xaca3246b, "platform_driver_unregister" },
	{ 0x39df0df0, "__platform_driver_register" },
	{ 0x7dd2df38, "platform_device_register" },
	{ 0xdcd1075b, "input_register_polled_device" },
	{ 0x1ce41bd2, "input_set_abs_params" },
	{ 0xd9ef27a6, "input_set_capability" },
	{ 0xfcbd64fb, "gpio_to_desc" },
	{ 0xbb17495b, "devm_gpio_request_one" },
	{ 0x1a7a6f3e, "devm_fwnode_get_index_gpiod_from_child" },
	{ 0x6ac6f4d, "devm_input_allocate_polled_device" },
	{ 0x3f0180cb, "fwnode_property_present" },
	{ 0x958192b8, "fwnode_property_read_string" },
	{ 0x28bd47a1, "fwnode_handle_put" },
	{ 0x4725c162, "fwnode_property_read_u32_array" },
	{ 0xdfb0134c, "device_get_next_child_node" },
	{ 0x5c8e47b3, "device_property_read_u32_array" },
	{ 0x28e7c3a8, "device_property_present" },
	{ 0xcf5e63f4, "devm_kmalloc" },
	{ 0x5b20cf64, "device_get_child_node_count" },
	{ 0xc0a3d105, "find_next_bit" },
	{ 0xb352177e, "find_first_bit" },
	{ 0xef60b10, "_dev_err" },
	{ 0xf9dc3c5, "gpiod_get_value_cansleep" },
	{ 0x584e594c, "input_event" },
	{ 0x7c32d0f0, "printk" },
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0xbdfb6dbb, "__fentry__" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=input-polldev";

MODULE_ALIAS("of:N*T*Cgpio-keys-polled");
MODULE_ALIAS("of:N*T*Cgpio-keys-polledC*");
