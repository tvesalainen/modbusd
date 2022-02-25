
#ifndef MODBUSTCP_H
#define MODBUSTCP_H

#include <linux/types.h>

#include "plugin.h"

#define ERROR_CODE(func) (func|0x80)

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
	__u16 quantity;
	__u8  bytes;
	struct modbus_arg data;
}__attribute__((packed));

struct modbus_rsp
{
	__u8  function;
	__u8  bytes;
	struct modbus_arg data;
}__attribute__((packed));

struct modbus_err
{
	__u8 error_code;
	__u8 exception_code;
}__attribute__((packed));

void *modbus(void *);

#endif /* MODBUSTCP_H */
