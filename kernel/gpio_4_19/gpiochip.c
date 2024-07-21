
#define DEBUG    1
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/gpio/driver.h>
#include <linux/platform_device.h>
#include <linux/gpio/machine.h>
#include <linux/gpio/consumer.h>
#include <linux/cdev.h>

static struct platform_device simple_platform_device = {
	.name = "simple_platform_device",
};

struct input_dev* simple_input;
int val = 0;
// 定义定时器结构体
static struct timer_list my_timer;

struct gpio_device {
	int			id;
	struct device		dev;
	struct cdev		chrdev;
	struct device		*mockdev;
	struct module		*owner;
	struct gpio_chip	*chip;
	struct gpio_desc	*descs;
	int			base;
	u16			ngpio;
	const char		*label;
	void			*data;
	struct list_head        list;

#ifdef CONFIG_PINCTRL
	/*
	 * If CONFIG_PINCTRL is enabled, then gpio controllers can optionally
	 * describe the actual pin range which they serve in an SoC. This
	 * information would be used by pinctrl subsystem to configure
	 * corresponding pins for gpio usage.
	 */
	struct list_head pin_ranges;
#endif
};

struct gpio_chip virtual_gc;
struct device virtual_dev;

void* gpio_data_reg;
void* gpio_set_reg;
void* gpio_dirout_reg;


struct gpiod_lookup_table gpios_table = {
	.dev_id = "virtual_dev",
	.table = {
		GPIO_LOOKUP_IDX("virtual_dev", 0, "test", 0, GPIO_ACTIVE_HIGH),
		{ },
	},
};



// 定时器处理函数
void my_timer_handler(struct timer_list *t)
{
		val = !val;

		printk(KERN_ALERT "my_timer_handler enter\r\n");
		// 重新启动定时器，如果需要的话
		input_report_key(simple_input , KEY_0, val);
		input_sync(simple_input);
		mod_timer(&my_timer, jiffies + msecs_to_jiffies(5000));
}

static int simple_request(struct gpio_chip *chip, unsigned gpio_pin)
{
	return 0;
}

void simple_free(struct gpio_chip *chip , unsigned offset)
{
	return 0;
}



// 模块初始化函数
static int __init simple_input_init(void) 
{
	int ret = 0;
	int value_gpio = 0;
	printk(KERN_ALERT "simple_input_init\r\n");
	simple_input = input_allocate_device(); 	/* 申请input_dev */ 
	simple_input->name = "simple_input"; 				/* 设置input_dev名字 */ 
	simple_input->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
	input_set_capability(simple_input , EV_KEY, KEY_0);
	

	dev_set_name(&virtual_dev, "virtual_dev");
	device_initialize(&virtual_dev);
	device_add(&virtual_dev);
	gpio_data_reg = kzalloc(4,GFP_KERNEL);
	gpio_set_reg = kzalloc(4,GFP_KERNEL);
	gpio_dirout_reg = kzalloc(4,GFP_KERNEL);

	memset(&virtual_gc , 0 , sizeof(virtual_gc));
	ret = bgpio_init(&virtual_gc, &virtual_dev, 4,
			 gpio_data_reg,
			 NULL , NULL,
			 gpio_dirout_reg, NULL,
			 0);
	virtual_gc.request = simple_request;
	virtual_gc.free = simple_free;
	virtual_gc.base = 400;

	printk(KERN_ALERT "ret = %d\r\n" , ret);
	if(ret)
		printk(KERN_ALERT "ret = %d , input_register_device fail\r\n" , ret);

	ret = devm_gpiochip_add_data(&virtual_dev , &virtual_gc , NULL);
	//ret = gpiochip_add_data(&virtual_gc , NULL);
	printk(KERN_ALERT "devm_gpiochip_add_data ret = %d\r\n" , ret);
	gpiod_add_lookup_table(&gpios_table);

	printk(KERN_ALERT "virtual_gc.base = %d\r\n" , virtual_gc.base);


	struct gpio_desc* simple_desc;
	simple_desc = gpiod_get(&virtual_dev, "test", GPIOD_OUT_HIGH);
	printk(KERN_ALERT "simple_desc = 0x%x\r\n" , simple_desc);
	
	printk(KERN_ALERT "virtual_gc.gpiodev->descs = 0x%x\r\n" , virtual_gc.gpiodev->descs);
	gpiod_set_value(simple_desc , 0);
	value_gpio = gpiod_get_value(simple_desc);
	printk(KERN_ALERT "value_gpio = %d\r\n" , value_gpio);

	gpiod_set_value(simple_desc , 1);
	value_gpio = gpiod_get_value(simple_desc);
	printk(KERN_ALERT "value_gpio = %d\r\n" , value_gpio);

	gpiod_put(simple_desc);
	

#if 0
	ret = input_register_device(simple_input);
	if(ret)
		printk(KERN_ALERT "input_register_device fail\r\n");
#endif

	//timer_setup(&my_timer, my_timer_handler, 0);
	//mod_timer(&my_timer, jiffies + msecs_to_jiffies(1000));


	//input_report_key(simple_input , KEY_0, 0);
	//input_sync(simple_input);

	return ret;
}
 
// 模块清理函数
static void __exit simple_input_exit(void) 
{
	printk(KERN_ALERT "simple_input_exit\r\n");
	//input_unregister_device(simple_input); /* 注销input_dev */ 
	input_free_device(simple_input); /* 删除input_dev */
	kfree(gpio_data_reg);
	kfree(gpio_set_reg);
	kfree(gpio_dirout_reg);
	//del_timer(&my_timer); 
	gpiod_remove_lookup_table(&gpios_table);
	devm_gpiochip_remove(&virtual_dev , &virtual_gc);
	device_del(&virtual_dev);

}
 
module_init(simple_input_init);
module_exit(simple_input_exit);

MODULE_LICENSE("GPL");
