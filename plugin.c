
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <arpa/inet.h>

#include "plugin.h"

int size;
__u16 *memory;

int init(const char* arg)
{
	fprintf(stderr, "init arg %s\n", arg);
	size = atoi(arg);
	memory = calloc(1, size);
	if (memory == NULL)
	{
		perror(arg);
		return -1;
	}
	return 0;
}

int read_holding_registers(__u16 address, __u16 quantity, struct modbus_arg *data)
{
	int ii;

	if (address+2*quantity > size)
	{
		return ILLEGAL_DATA_ADDRESS;
	}
	for (ii=0;ii<quantity;ii++)
	{
		data->array_short[ii] = htons(memory[ii]);
	}
	return OK;
}

int write_multiple_registers(__u16 address, __u16 quantity, struct modbus_arg *data)
{
	int ii;

	if (address+2*quantity > size)
	{
		return ILLEGAL_DATA_ADDRESS;
	}
	for (ii=0;ii<quantity;ii++)
	{
		memory[ii] = ntohs(data->array_short[ii]);
	}
	return OK;
}
