#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/time.h>
#include <linux/timer.h>
#include "asm-generic/errno-base.h"
#include "asm-generic/gpio.h"
#include "asm/delay.h"
#include "asm/gpio.h"
#include "linux/err.h"
#include "linux/export.h"
#include "linux/gpio.h"
#include "linux/mod_devicetable.h"
#include "linux/of_address.h"
#include "linux/of_gpio.h"
#include "linux/platform_device.h"
#include "linux/printk.h"
#include "linux/string.h"
#include "linux/sysfs.h"
#include <linux/delay.h>

#define DEV_NAME "HCSR04"
static int gpio_trig;
static int gpio_echo;

int hcsr04_probe(struct platform_device *dev);
int hcsr04_remove(struct platform_device *dev);
ssize_t hcsr04_read(struct file *f, char __user *userdata, size_t size, loff_t *off);
int hcsr04_open(struct inode *inode, struct file *f);
int hcsr04_release(struct inode *inode, struct file *f);
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = hcsr04_open,
    .release = hcsr04_release,
    .read = hcsr04_read,
};
static struct miscdevice hcsr04_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME,
    .fops = &fops,
};
struct device_node *pnode;
static struct of_device_id of_match_table[] = {{.compatible = "pute,putehcsr04"}, {}};
static struct platform_device_id id_table[] = {{
                                                   .name = "putehcsr04",
                                               },
                                               {}};
struct platform_driver plat_drv = {
    .probe = hcsr04_probe,
    .remove = hcsr04_remove,
    .driver =
        {
            .name = "putehcsr04",
            .owner = THIS_MODULE,
            .of_match_table = of_match_table,
        },
    .id_table = id_table,
};
ssize_t hcsr04_read(struct file *f, char __user *userdata, size_t size, loff_t *off)
{
  ssize_t ret = 0;
  ktime_t start, end;
  pr_info("dadada\n");
  long long  delta_us = 0;
  gpio_direction_output(gpio_trig, 0);
  gpio_direction_input(gpio_echo);
  gpio_set_value(gpio_trig, 1);
  udelay(10);  // 10us
  gpio_set_value(gpio_trig, 0);
  while (!gpio_get_value(gpio_echo))
  {
    cpu_relax();
  }
  start = ktime_get();
  while (gpio_get_value(gpio_echo))
  {
    cpu_relax();
  }
  end = ktime_get();
  delta_us = ktime_to_us(ktime_sub(end, start));
  ret = copy_to_user(userdata, &delta_us, sizeof(delta_us));
  if (ret < 0)
  {
    pr_info("copy_to_user error\n");
    ret = 0;
  }
  ret = sizeof(delta_us);
   msleep(1000);  
  return ret;
}
int hcsr04_open(struct inode *inode, struct file *f)
{
  pr_info("myopen call\n");
  return 0;
}
int hcsr04_release(struct inode *inode, struct file *f)
{
  pr_info("my remove call\n");
  return 0;
}
int hcsr04_probe(struct platform_device *dev)
{
  int ret = 0;
  ret = misc_register(&hcsr04_misc);
  if (ret < 0)
  {
    pr_err("misc_register error errno: %d\n", ret);
    return ret;
  }
  pnode = of_find_node_by_path("/putehcsr04");
  if (NULL == pnode)
  {
    pr_err("of_find_node_by_path error ");
    return ENODEV;
    goto del_misc;
  }
  gpio_trig = of_get_named_gpio(pnode, "trig-gpio", 0);
  if (gpio_trig < 0)
  {
    pr_err("of_get_named_gpio error,errno:%d\n", gpio_trig);
    ret = gpio_trig;
    goto del_misc;
  }
  gpio_echo = of_get_named_gpio(pnode, "echo-gpio", 0);
  if (gpio_echo < 0)
  {
    pr_err("of_get_named_gpio error,errno:%d\n", gpio_echo);
    ret = gpio_echo;
    goto del_misc;
  }
  ret = devm_gpio_request(hcsr04_misc.this_device, gpio_trig, "gpio_trig");
  if (ret < 0)
  {
    pr_err("misc_register error errno: %d\n", ret);
    goto del_pnode;
  }
  ret = devm_gpio_request(hcsr04_misc.this_device, gpio_echo, "gpio_echo");
  if (ret < 0)
  {
    pr_err("misc_register error errno: %d\n", ret);
    goto del_pnode;
  }
  pr_info("pute led  myprobe ok\n");
  gpio_direction_output(gpio_trig, 0);
  return 0;
del_pnode:
  of_node_put(pnode);
del_misc:
  misc_deregister(&hcsr04_misc);
  return ret;
}
int hcsr04_remove(struct platform_device *dev)
{
  misc_deregister(&hcsr04_misc);
  return 0;
}

static int __init hcsr04_init(void)
{
  int ret = 0;
  ret = platform_driver_register(&plat_drv);
  if (ret < 0)
  {
    pr_err("platform_driver_register error errno: %d\n", ret);
    return ret;
  }
  return 0;
}
static void __exit hcsr04_exit(void)
{
  platform_driver_unregister(&plat_drv);
  return;
}
module_init(hcsr04_init);
module_exit(hcsr04_exit);
MODULE_LICENSE("GPL");


// int ret = read(fd, &a, sizeof(a));

// if(ret == sizeof(a))
// {
//     int cm = (a * 340LL) / 20000;

//     //HC‑SR04有效区间3~400厘米
//     if(cm >= 3 && cm <= 400)
//     {
//         printf("距离：%d cm\n", cm);
//     }
//     else
//     {
//         printf("-----无效读数-----\n");
//     }
// }
// sleep(2);