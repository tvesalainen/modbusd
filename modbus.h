/*
 * Copyright (C) 2022 Timo Vesalainen <timo.vesalainen@iki.fi>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
