
all: modbusd plugin.so gpio.so

modbusd: modbusd.o modbus.o
	gcc -pthread modbusd.o modbus.o -o modbusd -ldl

gpio.so:	gpio.o
	ld -shared -o gpio.so gpio.o -lc

plugin.so:	plugin.o
	ld -shared -o plugin.so plugin.o -lc

