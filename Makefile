#==========================================================
# 用户程序 app 顶层 Makefile
# 编译流程：
#   1. 编译 mqtt 子模块 -> mqtt/mqtt.o
#   2. 编译 dev 内核模块（adxl235/mq3/hcsr04）-> *.ko
#   3. 编译并链接用户程序 app
#   4. 拷贝 app 与 *.ko 到 nfs 根文件系统
#==========================================================

#---------------- 子模块路径 ------------------
APP_ADD    := ./mailbox/
DEV_ADD    := ./dev
MQTT_ADD   := ./mqtt
LVGL_ADD   := ./lv_port_linux_frame_buffer

#---------------- 编译器 / 编译选项 ------------
CC      := arm-linux-gnueabihf-gcc
CFLAGS  := -std=gnu99 -g -Wall
# 头文件搜索路径
CFLAGS  += -I./mbox
CFLAGS  += $(addprefix -I, $(APP_ADD))
CFLAGS  += -I$(MQTT_ADD)
# LVGL / GUI 头文件路径
CFLAGS  += -I$(LVGL_ADD)
CFLAGS  += -I$(LVGL_ADD)/lvgl
CFLAGS  += -I$(LVGL_ADD)/lv_drivers
CFLAGS  += -I$(LVGL_ADD)/gui/generated
CFLAGS  += -I$(LVGL_ADD)/gui/custom
CFLAGS  += -I$(LVGL_ADD)/gui/generated/guider_fonts
CFLAGS  += -I$(LVGL_ADD)/gui/generated/guider_customer_fonts

# LVGL 静态库
LVGL_LIB := $(LVGL_ADD)/liblvgl.a

#---------------- 链接选项 --------------------
# 链接库搜索路径
LDFLAGS  := -L../paho.mqtt.c-master/build/output
LDFLAGS  += -L../mqtt/openssl-1.0.0s/__install/lib
LDLIBS   := -lpaho-mqtt3c -lrt -lssl -lcrypto -pthread -lm

# NFS 根文件系统目标目录
NFS_DIR  := $(HOME)/nfs/rootfs

#---------------- 目标 ------------------------
all:
	# 1. 编译 mqtt 子模块，生成 mqtt/mqtt.o
	$(MAKE) -C $(MQTT_ADD) all
	# 2. 编译 dev 内核模块，生成 .ko 并拷贝到 NFS
	$(MAKE) -C $(DEV_ADD) all
	# 3. 编译 LVGL 界面库，生成 liblvgl.a
	$(MAKE) -C $(LVGL_ADD) all

	# 4. 链接所有源文件与 LVGL 库生成 app
	$(CC) $(CFLAGS) main.c dev.c $(APP_ADD)*.c $(MQTT_ADD)/mqtt.o \
	      $(LVGL_LIB) $(LDFLAGS) $(LDLIBS) -o app

	# 5. 拷贝 app 到 NFS 根文件系统
	cp -f ./app $(NFS_DIR)/

clean:
	$(MAKE) -C $(MQTT_ADD) clean
	$(MAKE) -C $(DEV_ADD) clean
	$(MAKE) -C $(LVGL_ADD) clean
	rm -f ./app

.PHONY: all clean
