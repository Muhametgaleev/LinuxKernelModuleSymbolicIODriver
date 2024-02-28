#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>

#define BUF_SIZE 2048
#define PARSE_ERROR 5

static DEFINE_MUTEX(mutex);

static dev_t first;
static struct cdev c_dev;
static struct class * cl;

struct list_res {
    struct list_res *head;
    struct list_res *last;
    struct list_res *prev;
    struct list_res *next;
    long result;
    long id;
};

struct list_res *head;

char ibuf[BUF_SIZE];
int id_counter = 1;

int my_stoi(int *n, char *s) {
    const int size_s = strlen(s);

    int sign = 0;
    int i = 0;
    if (s[0] == '+')
        i = 1;
    if (s[0] == '-') {
        i = 1;
        sign = 1;
    }

    int value = 0;
    for (; i < size_s; ++i) {
        if (s[i] < '0' || s[i] > '9') {
            return 0;
        }
        int digit = s[i] - '0';
        value = value * 10 + digit;
    }

    if (sign == 1)
        value = value * -1;

    *n = value;
    return 0;
}

int is_num(char symbol) {
    return symbol >= '0' && symbol <= '9' ? 1 : 0;
}

int is_char(char symbol) {
    return ((symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z')) ? 1 : 0;
}

int is_action(char symbol) {
    return (symbol == '&' || symbol == '|' || symbol == '^') ? 1 : 0;
}

int get_action(char action) {
    if (action == '&') return 0;
    if (action == '|') return 1;
    if (action == '^') return 2;
    return -1;
}

int calculate(int val1, int val2, char action) {
    int result;
    int act = get_action(action);

    if (act == 0) {
        result = val1 & val2;
    } else if (act == 1) {
        result = val1 | val2;
    } else if (act == 2) {
        result = val1 ^ val2;
    } else {
        result = -1;
    }

    return result;
}

void clear_buffer(void) {
    for (int i = 0; i < BUF_SIZE; i++) {
        ibuf[i] = '\x00';
    }
}

void clear_subbuffer(char *buf) {
    for (int i = 0; i < strlen(buf); i++) {
        if (!is_num(buf[i])) buf[i] = '\x00';
    }
}

void get_message_error(void) {
    clear_buffer();
    ibuf[0] = 'E';
    ibuf[1] = 'r';
    ibuf[2] = 'r';
    ibuf[3] = 'o';
    ibuf[4] = 'r';
}

void get_text_result(struct list_res *res, char *resp) {

    char *curr = kcalloc(32, sizeof(char), GFP_KERNEL);

    printk(KERN_INFO "Val2: %d", res->result);

    snprintf(curr, 32, "id = %ld: result = %ld\n", res->id, res->result);

    strcat(resp, curr);
    kfree(curr);
}

void get_text_list_result(struct list_res *list, char *resp) {

    if (list != NULL) {
        struct list_res *next;
        while (list != NULL) {
            next = list->prev;
            get_text_result(list, resp);
            list = next;
        }
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

    char action;
    char v1c[34], v2c[34];
    int flag = 0, length_val1 = 0, length_val2 = 0;

    for (int i = 0; i < sizeof(ibuf); i++) {
        char symbol = ibuf[i];

        if (symbol == '\x00') {
            break;
        }

        if (symbol == ' ') {
            continue;
        }

        if (is_char(symbol)) {
            get_message_error();
            return PARSE_ERROR;
        }

        if (is_action(symbol)) {
            action = symbol;
            flag = 1;
        }

        if (is_num(symbol)) {
            if (flag == 0) {
                v1c[length_val1++] = symbol;
            } else {
                v2c[length_val2++] = symbol;
            }
        }
    }

    clear_subbuffer(v1c);
    clear_subbuffer(v2c);

    int val1 = 0, val2 = 0;

    my_stoi(&val1, v1c);
    my_stoi(&val2, v2c);

    int result = calculate(val1, val2, action);

    if (id_counter == 1) {
        head = kmalloc(sizeof(struct list_res), GFP_KERNEL);

        head->id = id_counter++;
        head->result = result;

        head->prev = NULL;
        head->next = NULL;
        head->head = head;
        head->last = head;

    } else {
        struct list_res *curr = kmalloc(sizeof(struct list_res), GFP_KERNEL);

        curr->id = id_counter++;
        curr->result = result;

        curr->head = head->head;

        curr->prev = head->last;
        curr->next = NULL;

        head->last = curr;
    }


    char *resp = kcalloc(BUF_SIZE, sizeof(char), GFP_KERNEL);

    get_text_list_result(head->last, resp);

    sprintf(ibuf, "%s\n", resp);

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

