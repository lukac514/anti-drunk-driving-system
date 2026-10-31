#ifndef __DEV_H__
#define __DEV_H__

int mqtt_init();
void mqtt_deinit();
int mqtt_send(char * key, int value);

#endif