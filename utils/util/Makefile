CROSS_COMPILE?=arm-none-linux-gnueabi-
CC=$(CROSS_COMPILE)gcc
CFLAGS=-Wall -O3

SRCS=rhino_led.c rhino_pwr.c rhino_mon.c rhino_fan.c rhino_fmc.c
EXE=$(SRCS:.c=)

all: $(EXE)

rhino_led: rhino_led.c
	$(CC) $(CFLAGS) -o $@ $^

rhino_pwr: rhino_pwr.c
	$(CC) $(CFLAGS) -o $@ $^

rhino_mon: rhino_mon.c i2c.o ina219.o max1668.o
	$(CC) $(CFLAGS) -o $@ $^

rhino_fan: rhino_fan.c i2c.o tc654.o
	$(CC) $(CFLAGS) -o $@ $^

rhino_fmc: rhino_fmc.c i2c.o
	$(CC) $(CFLAGS) -o $@ $^
	
.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(EXE) *.o
