
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int adxl235_init()
{
  int fd_adxl235 = 0;
  char buf[10] = {0};
  fd_adxl235 = open("/dev/ptadxl345", O_RDWR);
  if (fd_adxl235 < 0)
  {
    perror("open");
    return 1;
  }
  return fd_adxl235;
}
int mq3_init()
{
  long long a;
  int fd_mq3 = 0;
  char buf[10] = {0};
  fd_mq3 = open("/dev/mq3", O_RDWR);
  if (fd_mq3 < 0)
  {
    perror("open");
    return 1;
  }
  return fd_mq3;
}
int hcsr04_init()
{
  long long a;
  int fd_hcsr04 = 0;
  char buf[10] = {0};
  fd_hcsr04 = open("/dev/putehcsr04", O_RDWR);
  if (fd_hcsr04 < 0)
  {
    perror("open");
    return 1;
  }
  return fd_hcsr04;
}

// int fd = 0;
//   char buf[10] = {0};
//   fd = open("/dev/ptadxl345", O_RDWR);
//   if (fd < 0)
//   {
//     perror("open");
//     return 1;
//   }

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
//   close(fd);
//   return 0;

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
//   close(fd);
//   return 0;
//}