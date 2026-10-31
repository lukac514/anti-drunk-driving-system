#==========================================================
# LVGL 静态库 Makefile (Gui-Guider 酒精检测界面)
# 产出 liblvgl.a，供 poject 顶层 Makefile 链接
#==========================================================

CC      := arm-linux-gnueabihf-gcc
AR      := arm-linux-gnueabihf-ar
CFLAGS  := -std=gnu99 -g -O2

# 本工程根目录
ROOT    := $(CURDIR)

# 头文件路径
INCS    := -I$(ROOT)
INCS    += -I$(ROOT)/lvgl
INCS    += -I$(ROOT)/lv_drivers
INCS    += -I$(ROOT)/gui/generated
INCS    += -I$(ROOT)/gui/custom
INCS    += -I$(ROOT)/gui/generated/guider_fonts
INCS    += -I$(ROOT)/gui/generated/guider_customer_fonts

# LVGL 核心源码（所有 src 子目录的 .c，排除 examples / demos）
LVGL_SRCS := $(wildcard $(ROOT)/lvgl/src/*/*.c)
LVGL_SRCS += $(wildcard $(ROOT)/lvgl/src/*/*/*.c)
LVGL_SRCS += $(wildcard $(ROOT)/lvgl/src/*/*/*/*.c)

# lv_drivers：只编 fbdev(显示) 和 evdev(触摸)
DRV_SRCS := $(ROOT)/lv_drivers/display/fbdev.c
DRV_SRCS += $(ROOT)/lv_drivers/indev/evdev.c

# GUI 生成代码 + 自定义代码 + 鼠标光标
GUI_SRCS := $(wildcard $(ROOT)/gui/generated/*.c)
GUI_SRCS += $(wildcard $(ROOT)/gui/generated/guider_fonts/*.c)
GUI_SRCS += $(ROOT)/gui/custom/custom.c
GUI_SRCS += $(ROOT)/mouse_cursor_icon.c

# 库目标
OBJS_SRC := $(LVGL_SRCS) $(DRV_SRCS) $(GUI_SRCS)
OBJS     := $(OBJS_SRC:.c=.o)

LIB := $(ROOT)/liblvgl.a

.PHONY: all clean

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@
	@echo "CC $<"

clean:
	rm -f $(OBJS) $(LIB)
