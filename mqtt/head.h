#ifndef HEAD_H
#define HEAD_H


#include <MQTTAsync.h>
#include <MQTTClient.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <string.h>
//#define OLD_ADDRESS     "tcp://218.201.45.7:1883"
#define NEW_ADDRESS     "tcp://4J9S0iyp4v.mqtts.acc.cmcconenet.cn:1883"
#define DEV_NAME    "YOUR_DEV_NAME"
#define CLIENTID "YOUR_CLIENTID"
#define PRODUCT_ID "YOUR_PRODUCT_ID"
#define PASSWD "YOUR_PASSWD"
#define QOS         0
#define TIMEOUT     10000L

//#define __cplusplus
#endif // HEAD_H
