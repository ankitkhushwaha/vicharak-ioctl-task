#include <linux/init.h>
#include <linux/module.h>
#include <linux/kfifo.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <linux/mutex.h>

#include "../ioctl_lkd.h"

#define DEVICE_NAME "ankit"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ANKIT");
MODULE_DESCRIPTION("the ioctl interface for circular buffer");

static int major = 0;

static struct class *cls;
static struct kfifo buff;

static DEFINE_MUTEX(kfifo_lock);
static DEFINE_MUTEX(buffer_lock);

static long ioctl_ops(struct file *filp, unsigned int cmd, unsigned long arg)
{
    pr_debug("i got called\n");
    int ret = 0;

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
        if (mutex_lock_interruptible(&kfifo_lock))
            return -ERESTARTSYS;

        if (!kfifo_initialized(&buff))
        {
            ret = kfifo_alloc(&buff, (int)arg * sizeof(struct data), GFP_KERNEL);
            if (ret < 0)
            {
                pr_err("kfifo_alloc failed\n");
                return ret;
            }
            pr_info("circular buffer initlized successfully with total element:%ld\n", arg);
        }
        else
            pr_info("circular buffer already initlized with total element:%d. \n", kfifo_size(&buff));

        mutex_unlock(&kfifo_lock);
        break;
    case IOCTL_LLKD_IOPUSH:
        pr_info("Push cmd executed\n");

        struct data __user *user_push = (struct data __user *)arg;
        struct data add_data;
        if (copy_from_user(&add_data, user_push, sizeof(struct data)))
        {
            pr_err("copy_from_user failed for struct data\n");
            return -EFAULT;
        }

        int tmp_len = add_data.length;
        if (tmp_len <= 0 || tmp_len > MAX_DATA)
        {
            pr_warn("Invalid data size given\n");
            return -EFAULT;
        }

        if (mutex_lock_interruptible(&kfifo_lock))
            return -ERESTARTSYS;
        if (kfifo_is_full(&buff))
        {
            pr_warn("circular buffer is full, no data pushed\n");
            mutex_unlock(&kfifo_lock);
            return -ENOSPC;
        }
        ret = kfifo_in(&buff, (void *)&add_data, sizeof(struct data));
        mutex_unlock(&kfifo_lock);

        pr_info("data: %.*s - len: %d inserted in kfifo with ret: %d.\n",
                add_data.length, add_data.data, add_data.length, ret);
        break;
    case IOCTL_LLKD_IOPOP:
        struct data rm_data;
        if (mutex_lock_interruptible(&kfifo_lock))
            return -ERESTARTSYS;
        if (kfifo_is_empty(&buff))
        {
            pr_warn("circular buffer is empty. No data poped\n");
            mutex_unlock(&kfifo_lock);
            return -ENODATA;
        }
        ret = kfifo_out(&buff, &rm_data, sizeof(struct data));
        mutex_unlock(&kfifo_lock);

        if (rm_data.length < 0 || rm_data.length > MAX_DATA)
            return -EIO;
        pr_info("data: %.*s - len: %d poped from kfifo successfully with ret: %d.\n",
                rm_data.length, rm_data.data, rm_data.length, ret);

        struct data __user *user_pop = (struct data __user *)arg;
        if (copy_to_user(user_pop, (void *)&rm_data, sizeof(struct data)))
        {
            pr_err("copy_to_user[pop ops] failed\n");
            return -EFAULT;
        }
        break;
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
    // kfifo_free(buff); // kiffio buff is not ptr
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);

    unregister_chrdev(major, DEVICE_NAME);
    pr_info("Device removed successfully\n");
}

module_init(kfifo_ioctl);
module_exit(kfifo_exit);
