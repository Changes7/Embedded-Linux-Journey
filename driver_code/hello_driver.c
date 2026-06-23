#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YourName");
MODULE_DESCRIPTION("A Simple Hello World Kernel Module for STM32MP157");
MODULE_VERSION("V1.0");


static  int  __init  hello_init(void){

    printk(KERN_INFO "======================================\n");
    printk(KERN_INFO "[Hello Driver] 模块加载成功！你好，STM32MP157 内核态！\n");
    printk(KERN_INFO "======================================\n");

    return 0;

}

static   void    __exit  hello_exit(void){

    printk(KERN_INFO "======================================\n");
    printk(KERN_INFO "[Hello Driver] 模块卸载成功！再见，内核态！\n");
    printk(KERN_INFO "======================================\n");


}
module_init(hello_init);
module_exit(hello_exit);
