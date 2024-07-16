#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/timer.h>

static struct platform_device simple_platform_device = {
	.name = "simple_platform_device",
};

struct input_dev* simple_input;
int val = 0;
// 定义定时器结构体
static struct timer_list my_timer;

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


// 模块初始化函数
static int __init simple_input_init(void) 
{
	int ret = 0;
	printk(KERN_ALERT "simple_input_init\r\n");
	simple_input = input_allocate_device(); 	/* 申请input_dev */ 
	simple_input->name = "simple_input"; 				/* 设置input_dev名字 */ 
	simple_input->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
	input_set_capability(simple_input , EV_KEY, KEY_0);
	


	ret = input_register_device(simple_input);
	if(ret)
		printk(KERN_ALERT "input_register_device fail\r\n");


	timer_setup(&my_timer, my_timer_handler, 0);
	mod_timer(&my_timer, jiffies + msecs_to_jiffies(1000));


	//input_report_key(simple_input , KEY_0, 0);
	//input_sync(simple_input);

	return ret;
}
 
// 模块清理函数
static void __exit simple_input_exit(void) 
{
	printk(KERN_ALERT "simple_input_exit\r\n");
	input_unregister_device(simple_input); /* 注销input_dev */ 
	input_free_device(simple_input); /* 删除input_dev */
	del_timer(&my_timer); 
}
 
module_init(simple_input_init);
module_exit(simple_input_exit);

MODULE_LICENSE("GPL");
