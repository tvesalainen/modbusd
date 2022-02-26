
#ifndef _MODBUSD_H
#define _MODBUSD_H

#include <syslog.h>

#define MODBUS_PORT 502

extern int verbose;

#define VERBOSE(...) if (verbose) fprintf(stderr, __VA_ARGS__)
#define ERROR(...)  syslog(LOG_USER|LOG_ERR, __VA_ARGS__)

struct plugin
{
	int (*read_holding_registers)(__u16, __u16, struct modbus_arg*);
	int (*write_multiple_registers)(__u16, __u16, struct modbus_arg*);
};

extern struct plugin *plugins[256];

#endif /* _MODBUSD_H */
