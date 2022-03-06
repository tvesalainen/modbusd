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

#ifndef _PLUGIN_H
#define _PLUGIN_H

#include <linux/types.h>
#include <syslog.h>

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

#define ERROR(...)  syslog(LOG_USER|LOG_ERR, __VA_ARGS__)

struct modbusd_ctx
{
	int verbose;
};

struct modbus_arg
{
	union
	{
		__u8 array_byte[250];
		__u16 array_short[125];	/* in network byte order - use htons and ntohs */
		__u32 array_int[62];	/* in network byte order - use htonl and ntohl */
	};
}__attribute__((packed));

int init(struct modbusd_ctx *ctx, const char* arg);
int read_holding_registers(int fd, __u16 address, __u16 quantity, struct modbus_arg *data);
int write_multiple_registers(int fd, __u16 address, __u16 quantity, struct modbus_arg *data);
 
#endif /* _PLUGIN_H */
