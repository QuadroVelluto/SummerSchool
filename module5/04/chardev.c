#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module for lab4");
MODULE_AUTHOR("Anton Shvedunov");

static char msg[100] = {0};
static short readPos = 0;
static int times = 0;
static int major;
static struct class *cls;

static int dev_open(struct inode *, struct file *);
static int dev_rls(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char __user *, size_t, loff_t *);

static struct file_operations fops =
	{
		.read = dev_read,
		.open = dev_open,
		.write = dev_write,
		.release = dev_rls,
};

static int __init sys_init(void)
{
	major = register_chrdev(0, "AntonChDev", &fops);
	cls = class_create("AntonClass");
	device_create(cls, NULL, MKDEV(major, 0), NULL, "AntonChDev");

	printk("Device registered with major %d\n", major);
	return 0;
}

static void __exit sys_exit(void)
{
	device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "AntonChDev");
	printk("Cleaning up...\n");
}

static int dev_open(struct inode *inod, struct file *fil)
{
	printk("Device opened %d times\n", ++times);
	return 0;
}

static ssize_t dev_read(struct file *flip, char __user *buff, size_t len, loff_t *off)
{
	short count = 0;
	while (len && (msg[readPos] != 0))
	{
		put_user(msg[readPos], buff++);
		count++;
		len--;
		readPos++;
	}
	return count;
}

static ssize_t dev_write(struct file *flip, const char __user *buff, size_t len, loff_t *off)
{
    ssize_t count;

    if (len > sizeof(msg) - 1)
        len = sizeof(msg) - 1;

    if (copy_from_user(msg, buff, len)) {
        return -EFAULT;
    }

    msg[len] = '\0';
    readPos = 0;
    count = len;

    return count;
}


static int dev_rls(struct inode *inod, struct file *fil)
{
	printk("Device closed\n");
	return 0;
}

module_init(sys_init);
module_exit(sys_exit);