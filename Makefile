
all: modbusd plugin.so gpio.so mcp342x.so

modbusd: modbusd.o modbus.o
	gcc -pthread modbusd.o modbus.o -o modbusd -ldl

gpio.so:	gpio.o
	ld -shared -o gpio.so gpio.o -lc

mcp342x.so:	mcp342x.o
	ld -shared -o mcp342x.so mcp342x.o -lc -li2c

plugin.so:	plugin.o
	ld -shared -o plugin.so plugin.o -lc

