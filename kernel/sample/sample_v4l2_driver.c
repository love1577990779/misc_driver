#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

// 模块初始化函数
static int __init hello_init(void) {
    printk(KERN_EMERG "hello world\r\n");
    return 0;
}
 
// 模块清理函数
static void __exit hello_exit(void) {
    printk(KERN_EMERG "hello exit\r\n");
}
 
module_init(hello_init);
module_exit(hello_exit);
 
MODULE_LICENSE("GPL");