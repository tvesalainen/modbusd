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

#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <arpa/inet.h>

#include "plugin.h"

int size;
__u16 *memory;

int init(struct modbusd_ctx *ctx, const char* arg)
{
	fprintf(stderr, "init arg %s\n", arg);
	size = atoi(arg);
	memory = calloc(size, sizeof(__u16));
	if (memory == NULL)
	{
		perror(arg);
		return -1;
	}
	return 0;
}

int read_holding_registers(int fd, __u16 address, __u16 quantity, struct modbus_arg *data)
{
	int ii;

	if (address+2*quantity > size)
	{
		return ILLEGAL_DATA_ADDRESS;
	}
	for (ii=0;ii<quantity;ii++)
	{
		data->array_short[ii] = htons(memory[address+ii]);
	}
	return OK;
}

int write_multiple_registers(int fd, __u16 address, __u16 quantity, struct modbus_arg *data)
{
	int ii;

	if (address+2*quantity > size)
	{
		return ILLEGAL_DATA_ADDRESS;
	}
	for (ii=0;ii<quantity;ii++)
	{
		memory[address+ii] = ntohs(data->array_short[ii]);
	}
	return OK;
}
