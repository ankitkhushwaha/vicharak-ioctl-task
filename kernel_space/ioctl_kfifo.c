#include <linux/init.h>
#include <linux/module.h>
#include <linux/kfifo.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/device.h>

#include "../ioctl_lkd.h"

#define DEVICE_NAME "ankit"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ANKIT");
MODULE_DESCRIPTION("the ioctl interface for circular buffer");

static int major = 0;

static struct class *cls;

static struct kfifo buff;

static long ioctl_ops(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
#define MAX_CHAR_SIZE 128
    struct data buffer;

    pr_debug("In ioctl method, cmd=%d\n", _IOC_NR(cmd));

    if (_IOC_TYPE(cmd) != IOCTL_LLKD_MAGIC)
    {
        pr_info("ioctl fail; magic #mismatch\n");
        return -ENOTTY;
    }
    if (_IOC_NR(cmd) > IOCTL_LLKD_MAXIOCTL)
    {
        pr_info("ioctl fail; invalid cmd?\n");
        return -ENOTTY;
    }
    switch (cmd)
    {
    case IOCTL_LLKD_IOSIZE:
        ret = kfifo_alloc(&buff, (int)arg, GFP_KERNEL);
        if (ret < 0)
        {
            pr_err("kfifo_alloc failed\n");
            return ret;
        }
        pr_info("circular buffer initlized successfully with size:%ld", arg);
        break;
    case IOCTL_LLKD_IOPUSH:
        ret = copy_from_user((void *)&buffer, (const void __user *)arg, sizeof(struct data));
        

        buffer = *(struct data *)arg;
        kfifo_in(&buff, buffer, sizeof(struct data));
        pr_debug("data: %s - len: %d inserted in kfifo successfully.\n", buffer.data, buffer.length);
        break;
    // case IOCTL_LLKD_IOPOP:
    //     kfifo_out(&buff, &buffer, sizeof(struct data));
    //     pr_debug("data: %s - len: %d poped in kfifo successfully.\n", buffer.data, buffer.length);
    //     break;
    default:
        return -ENOTTY;
    }
    return ret;
}

static const struct file_operations char_ops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = ioctl_ops,
};

static int __init kfifo_ioctl(void)
{
    int ret;
    struct device *dev;

    major = register_chrdev(major, DEVICE_NAME, &char_ops);
    if (major < 0)
    {
        pr_err("Registering char device failed with %d\n", major);
        return major;
    }
    pr_info("char driver assigned major number %d.\n", major);

    cls = class_create(DEVICE_NAME);
    if (IS_ERR(cls))
    {
        ret = PTR_ERR(cls);
        pr_err("class_create failed with err: %d", ret);
        goto out;
    }

    dev = device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(dev))
    {
        ret = PTR_ERR(dev);
        pr_err("class_create failed with err: %d", ret);
        goto out1;
    }
    pr_info("Device created on /dev/%s\n", DEVICE_NAME);
    return 0;

out1:
    class_destroy(cls);
out:
    unregister_chrdev(major, DEVICE_NAME);
    return ret;
}

static void __exit kfifo_exit(void)
{
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);

    unregister_chrdev(major, DEVICE_NAME);
    pr_info("Device removed successfully\n");
}

module_init(kfifo_ioctl);
module_exit(kfifo_exit);
