#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/sched.h>
#include <linux/security.h>
#include <linux/time.h>
#include <linux/timer.h>
#include "asm-generic/errno-base.h"
#include "asm-generic/gpio.h"
#include "asm/delay.h"
#include "asm/gpio.h"
#include "linux/device.h"
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

#define ADC_RAW_PATH "/sys/bus/iio/devices/iio\:device0/in_voltage1_raw"
#define ADC_SCALE_PATH "/sys/bus/iio/devices/iio\:device0/in_voltage_scale"
typedef struct
{
  char raw[32];
  char scale[32];
} MQ3;
static int mq3_probe(struct platform_device *pdev);
static int mq3_remove(struct platform_device *pdev);
static ssize_t mq3_read(struct file *file, char __user *userdata, size_t count, loff_t *ppos);
static MQ3 mq3 = {
    .raw = {0},
    .scale = {0},
};
static struct miscdevice mq3_misc_dev;
static struct file_operations mq3_fops = {
    .owner = THIS_MODULE,
    .read = mq3_read,
};

static const struct of_device_id mq3_of_match[] = {{.compatible = "pute,putemq3"}, {}};
MODULE_DEVICE_TABLE(of, mq3_of_match);
static struct platform_driver mq3_driver = {
    .probe = mq3_probe,
    .remove = mq3_remove,
    .driver =
        {
            .name = "putemq3",
            .of_match_table = mq3_of_match,
        },
};
ssize_t mq3_read(struct file *file, char __user *userdata, size_t count, loff_t *ppos)
{
  int ret;
  struct file *filp;
  loff_t pos = 0;
  memset(mq3.raw, 0, 32);
  memset(mq3.scale, 0, 32);

  filp = filp_open(ADC_RAW_PATH, O_RDONLY, 0);
  if (IS_ERR(filp))
  {
    return PTR_ERR(filp);
  }
  ret = kernel_read(filp, pos, mq3.raw, sizeof(mq3.raw) - 1);
  pr_info("%s\n", mq3.raw);

  filp_close(filp, NULL);
  if (ret < 0)
  {
    return ret;
  }
  filp = filp_open(ADC_SCALE_PATH, O_RDONLY, 0);
  if (IS_ERR(filp))
  {
    return PTR_ERR(filp);
  }
  ret = kernel_read(filp, pos, mq3.scale, sizeof(mq3.scale) - 1);

  filp_close(filp, NULL);
  if (ret < 0)
  {
    return ret;
  }
  pr_info("%s\n", mq3.scale);
  if (copy_to_user(userdata, &mq3, 64))
    return -EFAULT;
  *ppos += 64;
  return 64;
}
int mq3_probe(struct platform_device *pdev)
{
  int ret;
  mq3_misc_dev.minor = MISC_DYNAMIC_MINOR;
  mq3_misc_dev.name = "mq3";
  mq3_misc_dev.fops = &mq3_fops;
  ret = misc_register(&mq3_misc_dev);
  if (ret)
  {
    dev_err(&pdev->dev, "misc register failed\n");
    return ret;
  }
  dev_info(&pdev->dev, "register /dev/mq3 ok\n");
  return 0;
}
int mq3_remove(struct platform_device *pdev)
{
  misc_deregister(&mq3_misc_dev);
  return 0;
}
module_platform_driver(mq3_driver);
MODULE_LICENSE("GPL");


//   while (1)
//   {
//     int raw = 0;
//     float scale = 0.0;
//     ssize_t a = read(fd, &mq3, sizeof(mq3));
//     if (a != 64)
//     {
//       printf("WDNMD\n");
//       return -1;
//     }
//     printf("raw =%d, scale=%f\n", raw, scale);
//     raw = atoi(mq3.raw);
//     scale = atof(mq3.scale);
//     float vol = (raw * scale) / 1000.f;
//     printf("raw =%d, scale=%f, vol =%f\n", raw, scale, vol);
//     sleep(2);
//   };
//   close(fd);
//   return 0;
// }