
modbusd: modbusd.o modbus.o
	gcc -pthread modbusd.o modbus.o -o modbusd
