#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define PROC_NAME "lab2"
#define MAX_LEN 255

MODULE_LICENSE("MOPS");
MODULE_AUTHOR("Anton Shvedunov");
MODULE_DESCRIPTION("Module for lab 2");

static char *buffer;
static int len;
static int temp;

static ssize_t read_proc(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	if (count > temp)
		count = temp;
	temp = temp - count;
	copy_to_user(ubuf, buffer, count);

	if(count == 0)
        temp = len;
    return count;
}

static ssize_t write_proc(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
	if (count > MAX_LEN)
		count = MAX_LEN;

	copy_from_user(buffer, ubuf, count);

	len = count;
	temp = len;
	return len;
}

static const struct proc_ops proc_operations = {
	proc_read : read_proc,
	proc_write : write_proc,
};

static int __init proc_init(void)
{
	proc_create(PROC_NAME, 0666, NULL, &proc_operations);
	buffer = kmalloc(MAX_LEN, GFP_KERNEL);
	return 0;
}

static void __exit proc_cleanup(void)
{
	remove_proc_entry(PROC_NAME, NULL);
	kfree(buffer);
}

module_init(proc_init);	   // вызывается при загрузке модуля
module_exit(proc_cleanup); // вызывается при выгрузке модуляn