#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#define DEVICE_NAME     "my_device"
#define CLASS_NAME      "my_device_class"
#define BUFFER_SIZE     256

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded-Linux-Journey");
MODULE_DESCRIPTION("Character device driver template for STM32MP157");
MODULE_VERSION("1.0");

static dev_t dev_num;
static struct cdev my_cdev;
static struct class *dev_class;
static struct device *dev_device;

static char device_buffer[BUFFER_SIZE];
static int buffer_len;
static int open_count;
static DEFINE_MUTEX(device_mutex);

static int my_open(struct inode *inode, struct file *file)
{
    mutex_lock(&device_mutex);
    open_count++;
    mutex_unlock(&device_mutex);

    printk(KERN_INFO "[my_device] open(): device opened, open_count=%d\n", open_count);
    return 0;
}

static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    ssize_t ret;
    int read_len;

    if (*ppos >= buffer_len)
        return 0;

    read_len = min((int)count, buffer_len - (int)*ppos);

    mutex_lock(&device_mutex);
    if (copy_to_user(buf, device_buffer + *ppos, read_len)) {
        mutex_unlock(&device_mutex);
        return -EFAULT;
    }
    mutex_unlock(&device_mutex);

    *ppos += read_len;
    ret = read_len;

    printk(KERN_INFO "[my_device] read(): %zd bytes\n", ret);
    return ret;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    ssize_t ret;
    int write_len;

    if (*ppos >= BUFFER_SIZE)
        return -ENOSPC;

    write_len = min((int)count, BUFFER_SIZE - (int)*ppos);

    mutex_lock(&device_mutex);
    if (copy_from_user(device_buffer + *ppos, buf, write_len)) {
        mutex_unlock(&device_mutex);
        return -EFAULT;
    }
    mutex_unlock(&device_mutex);

    *ppos += write_len;
    if (*ppos > buffer_len)
        buffer_len = *ppos;

    ret = write_len;
    printk(KERN_INFO "[my_device] write(): %zd bytes, buffer_len=%d\n", ret, buffer_len);
    return ret;
}

static int my_release(struct inode *inode, struct file *file)
{
    mutex_lock(&device_mutex);
    open_count--;
    mutex_unlock(&device_mutex);

    printk(KERN_INFO "[my_device] release(): device closed, open_count=%d\n", open_count);
    return 0;
}

static const struct file_operations my_fops = {
    .owner   = THIS_MODULE,
    .open    = my_open,
    .read    = my_read,
    .write   = my_write,
    .release = my_release,
};

static int __init my_device_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "[my_device] alloc_chrdev_region failed: %d\n", ret);
        return ret;
    }

    cdev_init(&my_cdev, &my_fops);
    my_cdev.owner = THIS_MODULE;

    ret = cdev_add(&my_cdev, dev_num, 1);
    if (ret < 0) {
        printk(KERN_ERR "[my_device] cdev_add failed: %d\n", ret);
        goto err_unregister;
    }

    dev_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(dev_class)) {
        ret = PTR_ERR(dev_class);
        printk(KERN_ERR "[my_device] class_create failed: %d\n", ret);
        goto err_cdev_del;
    }

    dev_device = device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(dev_device)) {
        ret = PTR_ERR(dev_device);
        printk(KERN_ERR "[my_device] device_create failed: %d\n", ret);
        goto err_class_destroy;
    }

    buffer_len = 0;
    open_count = 0;
    memset(device_buffer, 0, BUFFER_SIZE);

    printk(KERN_INFO "======================================\n");
    printk(KERN_INFO "[my_device] driver loaded\n");
    printk(KERN_INFO "[my_device] major=%d, minor=%d\n",
           MAJOR(dev_num), MINOR(dev_num));
    printk(KERN_INFO "[my_device] device node: /dev/%s\n", DEVICE_NAME);
    printk(KERN_INFO "======================================\n");
    return 0;

err_class_destroy:
    class_destroy(dev_class);
err_cdev_del:
    cdev_del(&my_cdev);
err_unregister:
    unregister_chrdev_region(dev_num, 1);
    return ret;
}

static void __exit my_device_exit(void)
{
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO "======================================\n");
    printk(KERN_INFO "[my_device] driver unloaded\n");
    printk(KERN_INFO "======================================\n");
}

module_init(my_device_init);
module_exit(my_device_exit);
