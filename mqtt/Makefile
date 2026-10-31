
DST=mqtt.o
SRC=mqtt.c
CC=arm-linux-gnueabihf-gcc
FLAG=-g
INCS = -I../paho.mqtt.c-master/src

$(DST): $(SRC)
	$(CC) $(FLAG) $(INCS) -c $(SRC) -o $(DST)

.PHONY: all clean
all: $(DST)

clean:
	rm -f $(DST)