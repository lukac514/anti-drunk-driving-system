# 嵌入式 Linux 多线程与图形界面项目

基于 C 语言的嵌入式 Linux 项目，代码包含设备驱动、线程间消息传递、MQTT 通信和 LVGL 图形界面。

## 目录

- `dev/`：ADXL235、MQ3、HC-SR04 等设备相关代码。
- `mailbox/`：线程消息队列、邮箱和网络相关代码。
- `mqtt/`：MQTT 连接与消息发布。
- `lv_port_linux_frame_buffer/`：LVGL、Linux framebuffer 驱动和界面代码。
- `main.c`、`dev.c`：应用入口与设备相关逻辑。

## 构建说明

项目 Makefile 面向 ARM Linux 交叉编译环境，依赖 LVGL、Paho MQTT C、OpenSSL、目标内核及其开发环境；部分依赖和 NFS 路径需根据本地环境调整。`mqtt/head.h` 中的连接凭据已替换为占位符，使用前需要自行配置。尚未在当前环境验证硬件运行。
