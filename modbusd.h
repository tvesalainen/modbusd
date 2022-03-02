
#ifndef _MODBUSD_H
#define _MODBUSD_H

#define MODBUS_PORT 502

extern int verbose;

struct plugin
{
	int (*read_holding_registers)(int, __u16, __u16, struct modbus_arg*);
	int (*write_multiple_registers)(int, __u16, __u16, struct modbus_arg*);
};

extern struct plugin *plugins[256];

#endif /* _MODBUSD_H */
