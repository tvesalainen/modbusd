
modbusd: modbusd.o modbus.o
	gcc -ldl -pthread modbusd.o modbus.o -o modbusd

plugin.so:	plugin.o
	ld -shared -o plugin.so plugin.o -lc

