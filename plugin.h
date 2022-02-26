
#ifndef _PLUGIN_H
#define _PLUGIN_H

#include <linux/types.h>

#define OK 0
#define ILLEGAL_FUNCTION 1
#define ILLEGAL_DATA_ADDRESS 2
#define ILLEGAL_DATA_VALUE 3
#define SLAVE_DEVICE_FAILURE 4
#define ACKNOWLEDGE 5
#define SLAVE_DEVICE_BUSY 6
#define MEMORY_PARITY_ERROR 8
#define GATEWAY_PATH_UNAVAILABLE 0xa
#define GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND 0xb

struct modbus_arg
{
	union
	{
		__u8 array_byte[250];
		__u16 array_short[125];	/* in network byte order - use htons and ntohs */
		__u32 array_int[62];	/* in network byte order - use htonl and ntohl */
	};
}__attribute__((packed));

int init(const char* arg);
int read_holding_registers(__u16 address, __u16 quantity, struct modbus_arg *data);
int write_multiple_registers(__u16 address, __u16 quantity, struct modbus_arg *data);
 
#endif /* _PLUGIN_H */
