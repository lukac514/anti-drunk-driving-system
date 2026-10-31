#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/sched.h>
#include <linux/spi/spi.h>
#include "asm-generic/errno-base.h"
#include "asm-generic/gpio.h"
#include "asm/gpio.h"
#include "linux/device.h"
#include "linux/err.h"
#include "linux/export.h"
#include "linux/gpio.h"
#include "linux/mod_devicetable.h"
#include "linux/of_address.h"
#include "linux/of_gpio.h"
#include "linux/of_irq.h"
#include "linux/platform_device.h"
#include "linux/printk.h"
#include "linux/string.h"
#include "linux/sysctl.h"
#include "linux/sysfs.h"
#include "linux/wait.h"
#include "linux/workqueue.h"
struct spi_device *pspi;
struct of_device_id of_table[] = {
    {
        .compatible = "pt,adxl345",
    },
    {},
};
struct spi_device_id id_table[] = {
    {.name = "adxl345"},
    {},
};
int myprobe(struct spi_device *spi);
int myremove(struct spi_device *spi);
ssize_t myread(struct file *f, char __user *userdata, size_t size, loff_t *off);
int myopen(struct inode *inode, struct file *f);
int myrelease(struct inode *inode, struct file *f);
static struct file_operations fops = {
    .read = myread,
    .owner = THIS_MODULE,
    .open = myopen,
    .release = myrelease,
};
struct miscdevice mymisc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "ptadxl345",
    .fops = &fops,
};

static struct spi_driver dev = {.probe = myprobe,
                                .remove = myremove,
                                .id_table = id_table,
                                .driver = {
                                    .name = "adxl345",
                                    .owner = THIS_MODULE,
                                    .of_match_table = of_table,
                                }};
int myopen(struct inode *inode, struct file *f)
{
  char txbuf[2] = {0x31, 0x08};
  char rxbuf[2] = {0};
  spi_write_then_read(pspi, txbuf, 2, rxbuf, 0);

  txbuf[0] = 0x2c;
  txbuf[1] = 0x0a;
  spi_write_then_read(pspi, txbuf, 2, rxbuf, 0);

  txbuf[0] = 0x2d;
  txbuf[1] = 0x08;
  spi_write_then_read(pspi, txbuf, 2, rxbuf, 0);
  pr_info("myopen ok\n");
  return 0;
}
int myrelease(struct inode *inode, struct file *f)
{
  pr_info("myrelease ok\n");
  return 0;
}
ssize_t myread(struct file *f, char __user *userdata, size_t size, loff_t *off)
{
  int ret = 0;
  char reg_add = 0x32 | 0x80 | 0x40;
  char rxbuf[6] = {0};
  spi_write_then_read(pspi, &reg_add, 1, rxbuf, 6);
  ret = copy_to_user(userdata, rxbuf, sizeof(rxbuf));
  if (ret)
  {
    pr_err("copy_to_user error\n");
    return ret;
  }
  pr_info("%s\n", rxbuf[0]);
  return sizeof(rxbuf);
}
int myprobe(struct spi_device *spi)
{
  int ret = 0;
  pspi = spi;
  ret = misc_register(&mymisc);
  if (ret)
  {
    pr_err("misc_register error %d\n", ret);
    return ret;
  }

  pr_info("adxl345 OK\n");
  return 0;
}
int myremove(struct spi_device *spi)
{
  misc_deregister(&mymisc);
  return 0;
}

module_spi_driver(dev);
MODULE_LICENSE("GPL");


//   while (1)
//   {
//     int ret = read(fd, buf, 6);
//     if (ret < 0)
//     {
//       perror("read");
//       break;
//     }
//     short adx1_x = buf[1] << 8 | buf[0];
//     short adx1_y = buf[3] << 8 | buf[2];
//     short adx1_z = buf[5] << 8 | buf[4];
    
//     printf("X :%d  ,Y : %d , Z : %d .\n", adx1_x, adx1_y, adx1_z);
//     sleep(1);
//   }