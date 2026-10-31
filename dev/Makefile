KER_ADD :=/home/l/imx6ull/linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek
CUR_ADD:=$(shell pwd)
obj-m+=adxl235.o mq3.o hcsr04.o

all:
	$(MAKE) -C $(KER_ADD) M=$(CUR_ADD) modules
	cp *.ko ~/nfs/rootfs

clean:
	$(MAKE) -C $(KER_ADD) M=$(CUR_ADD) clean
	rm ~/nfs/rootfs/*.ko