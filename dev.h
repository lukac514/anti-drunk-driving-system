#ifndef __DEV_H__
#define __DEV_H__
typedef struct
{
  char raw[32];
  char scale[32];
} MQ3;
static MQ3 mq3 = {
    .raw = {0},
    .scale = {0},
};
int adxl235_init();
int mq3_init();
int hcsr04_init();
#endif