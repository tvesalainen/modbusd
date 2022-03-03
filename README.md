# Modbusd a simple modbus tcp server
Implements read_holding_registers (03) and write_multiple_registers (16).
Units are implemented as dynamic libraries.
Linux GPIO is supported.
# Installation
```
git clone https://github.com/tvesalainen/modbusd.git
cd modbusd
make
debuild -us -uc
sudo dpkg -i ../modbusd_0.1_XXXXX.deb
```
After that you have modbus tcp server running with GPIO support at unit 0.
## Command line
```
./modbusd: invalid option -- 'h'
usage:
        -v                      verbose
        -s                      log only to syslog
        -p <port>               port
        -l <unit>:<lib>:<arg>   add plugin
        -h                      help
```

### Verbose
Extra text is written to stderr
### Port
Modbus port is 502. It is possible to change with -p port.
### Plugin
Add plugin with -l.
Example
```
modbusd -l 0:/usr/lib/gpio.so:/dev/gpiochip0

0 is unit. (0 - 255)
/usr/lib/gpio.so is path to dynamic library
/dev/gpiochip0 is library argument
```
There can be several libraries
## Implementing a unit (library)
See plugin.c and gpio.c and Makefile for compiling.
## GPIO
16 bit address lowest 8 bits are used as gpio line offset.
If address high 8 bits are 1 read is polled. In other words read blocks until gpio pin changes
status.

Example: read line 14
```
read 0x0 0xe // normal read

read 0x0 0xe|0x100 // blocking read


```

