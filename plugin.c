
#include <linux/types.h>
#include <arpa/inet.h>

#include "plugin.h"

int read_holding_registers(__u16 address, __u16 quantity, struct modbus_arg *data)
{
	int ii;

	for (ii=0;ii<quantity;ii++)
	{
		data->array_short[ii] = htons(address);
	}
	return OK;
}
