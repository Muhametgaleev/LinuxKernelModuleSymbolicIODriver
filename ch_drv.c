#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define BUF_SIZE 256

static DEFINE_MUTEX(mutex);

static dev_t first;
static struct cdev c_dev;
static struct class * cl;

char ibuf[BUF_SIZE];
int counter = 0;

int check_cond(char symbol) {
    return symbol != '\n' && symbol != '\r' ? 1 : 0;
}

void clear_buffer(void) {
    for (int i = 0; i < BUF_SIZE; i++) {
        ibuf[i] = '\x00';
    }
}

static int my_open(struct inode *i, struct file *f)
{
    mutex_lock(&mutex);
    printk(KERN_INFO "Driver: file open()\n");
    return 0;
}

static int my_close(struct inode *i, struct file *f)
{
    mutex_unlock(&mutex);
    printk(KERN_INFO "Driver: file close()\n");
    return 0;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "Driver: start reading\n");

    int count = strlen(ibuf);

     if (*off > 0 || len < count) {
       return 0;
     }

     if (copy_to_user(buf, ibuf, count) != 0) {
       return -EFAULT;
    }

    *off = count;

    printk(KERN_INFO "Driver: finish reading\n");
    printk(KERN_INFO "Symbols counter: %d", counter);
    return count;
}

static ssize_t my_write(struct file *f, const char __user *buf,  size_t len, loff_t *off)
{
    printk(KERN_INFO "Driver: start writing()\n");

    if (len > BUF_SIZE) {
        return 0;
    }

    clear_buffer();
    if (copy_from_user(ibuf, buf, len) != 0) {
         return -EFAULT;
    }

    for (int i = 0; i < sizeof(ibuf); i++) {
        char symbol = ibuf[i];
        if (symbol == '\x00') break;
        //printk(KERN_INFO "Symbol: %c", ibuf[i]);
        if (check_cond(symbol)) {
            counter++;
        }
    }

    printk(KERN_INFO "Driver: finish writing()\n");
    return len;
}

static struct file_operations mychdev_fops =
{
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .write = my_write
};

static int __init ch_drv_init(void)
{
    if (alloc_chrdev_region(&first, 0, 1, "ch_dev") < 0)
	  {
		return -1;
	  }
    if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL)
	  {
		unregister_chrdev_region(first, 1);
		return -1;
	  }

      if (device_create(cl, NULL, first, NULL, "mychdev") == NULL)
      {
          class_destroy(cl);
          unregister_chrdev_region(first, 1);
          return -1;
      }


      cdev_init(&c_dev, &mychdev_fops);
    if (cdev_add(&c_dev, first, 1) == -1)
	  {
		device_destroy(cl, first);
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	  }

    printk(KERN_INFO "Hello!\n");

    return 0;
}

static void __exit ch_drv_exit(void)
{
    cdev_del(&c_dev);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);

    printk(KERN_INFO "Bye!!!\n");
}

module_init(ch_drv_init);
module_exit(ch_drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Author");
MODULE_DESCRIPTION("The first kernel module");

