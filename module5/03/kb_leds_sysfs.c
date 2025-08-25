#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/tty.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/console_struct.h>
#include <linux/timer.h>
#include <linux/vt_kern.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anton Shvedunov");
MODULE_DESCRIPTION("Module for lab3");

static struct kobject *example_kobject;
static int test;
static struct timer_list my_timer;
static struct tty_driver *my_driver;
static int _kbledstatus = 0;

#define BLINK_DELAY HZ / 5
#define ALL_LEDS_ON 0x07
#define RESTORE_LEDS 0x00

static void my_timer_func(struct timer_list *ptr)
{
	if (test != 0)
	{
		if (_kbledstatus == test)
			_kbledstatus = RESTORE_LEDS;
		else
			_kbledstatus = test;

		(my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty, KDSETLED, _kbledstatus);

		my_timer.expires = jiffies + BLINK_DELAY;
		add_timer(&my_timer);
	}
	else
	{
		(my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);
	}
}

static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", test);
}

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int old_value = test;
	sscanf(buf, "%du", &test);

	if (test != 0 && old_value == 0)
	{
		my_timer.expires = jiffies + BLINK_DELAY;
		add_timer(&my_timer);
	}
	else if (test == 0 && old_value != 0)
	{
		timer_delete_sync(&my_timer);
		(my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);
	}

	return count;
}

static struct kobj_attribute foo_attribute = __ATTR(test, 0660, foo_show, foo_store);

static int __init sys_init(void)
{
	int error = 0;

	printk("Module initialized successfully \n");

	my_driver = vc_cons[fg_console].d->port.tty->driver;
	timer_setup(&my_timer, my_timer_func, 0);

	example_kobject = kobject_create_and_add("systest", kernel_kobj);
	if (!example_kobject)
		return -ENOMEM;

	error = sysfs_create_file(example_kobject, &foo_attribute.attr);
	if (error)
	{
		printk("failed to create the foo file in /sys/kernel/systest \n");
		kobject_put(example_kobject);
		return error;
	}

	return error;
}

static void __exit sys_exit(void)
{
	printk("Module uninitialized successfully \n");

	timer_delete(&my_timer);
	(my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);

	kobject_put(example_kobject);
}

module_init(sys_init);
module_exit(sys_exit);