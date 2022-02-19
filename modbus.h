
#ifndef MODBUSTCP_H
#define MODBUSTCP_H

#include <linux/types.h>

struct modbus_tcp
{
	__u16 transaction_id;
	__u16 protocol_id;	/* 0 = MODBUS protocol */
	__u16 len;		/* Number of following bytes */
	__u8  unit_id;
}__attribute__((packed));

struct modbus_req
{
	__u8  function;
	__u16 address;
	__u16 registers;
	__u8  bytes;
	__u16 data[123];
}__attribute__((packed));

void *modbus(int *);

#endif /* MODBUSTCP_H */
