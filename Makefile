
modbusd: modbusd.o modbus.o
	gcc -ldl -pthread modbusd.o modbus.o -o modbusd

gpio.so:	gpio.o
	ld -shared -o gpio.so gpio.o -lc

plugin.so:	plugin.o
	ld -shared -o plugin.so plugin.o -lc

